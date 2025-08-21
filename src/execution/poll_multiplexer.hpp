/* Copyright 2025 Kevin Exton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifndef IO_POLL_MULTIPLEXER_HPP
#define IO_POLL_MULTIPLEXER_HPP
#include "detail/execution_concepts.hpp"
#include "detail/immovable.hpp"
#include <error.hpp>
#include <io.hpp>
#include <socket/socket_handle.hpp>

#include <stdexec/execution.hpp>

#include <algorithm>
#include <chrono>
#include <ios>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <system_error>

#include <poll.h>

namespace io::execution {
class poll_multiplexer {

public:
  using size_type = std::size_t;
  using interval_type = std::chrono::milliseconds;
  using event_base = struct pollfd;
  struct event_type : public event_base {
    [[nodiscard]] constexpr auto key() const -> int { return fd; }
  };
  using interest_list = std::list<event_type>;

  struct op_ : detail::immovable {
    op_() = default;
    op_(const op_ &) = default;
    auto operator=(const op_ &) -> op_ & = default;
    op_(op_ &&) = delete;
    auto operator=(op_ &&) -> op_ & = delete;

    event_type *event{};
    interest_list *interest{};
    std::mutex *mtx{};

    virtual auto complete() -> void = 0;

    virtual ~op_() = default;
  };

  struct op_queues_ {
    std::queue<op_ *> read_queue;
    std::queue<op_ *> write_queue;
  };
  using events_type = std::map<decltype(event_type{}.key()), op_queues_>;

  template <typename Fn>
    requires std::is_invocable_v<Fn>
  static auto lock_exec(std::unique_lock<std::mutex> lock,
                        Fn func) -> decltype(auto) {
    return func();
  }

  template <typename Receiver, detail::Operation<event_type> Fn>
  struct poll_op : public op_ {
    Receiver receiver;
    events_type *events{};
    Fn &&func;

    auto complete() -> void override {
      std::lock_guard lock{*mtx};

      std::streamsize len = func(*event);
      if (len >= 0) {
        stdexec::set_value(std::move(receiver), len);
      } else {
        stdexec::set_error(std::move(receiver), errno);
      }
    }

    auto start() -> void {
      std::lock_guard lock{*mtx};

      auto [qit, emplaced] = events->try_emplace(event->key());
      auto &queue = (event->events & POLLIN) ? qit->second.read_queue
                                             : qit->second.write_queue;
      queue.push(this);
    }
  };

  template <detail::Operation<event_type> Fn> struct poll_sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    template <typename Receiver>
    auto connect(Receiver receiver) -> poll_op<Receiver, Fn> {
      std::lock_guard lock{*mtx};

      auto event_it = std::lower_bound(
          std::begin(*interest), std::end(*interest), event.fd,
          [&](const auto &lhs, int rhs) { return lhs.fd < rhs; });

      if (event_it != std::end(*interest) && event_it->fd == event.key()) {
        event_it->events |= event.events;
      } else {
        event_it = interest->insert(event_it, event);
      }

      return {.receiver = std::move(receiver),
              .func = std::move(func),
              .event = &*event_it,
              .interest = interest,
              .events = events,
              .mtx = mtx};
    }

    Fn func;
    event_type event;
    interest_list *interest;
    events_type *events;
    std::mutex *mtx;
  };

  template <detail::Operation<event_type> Fn>
  auto submit(event_type event, Fn func) -> poll_sender<Fn> {
    return {.func = std::move(func),
            .event = event,
            .interest = &interest_,
            .events = &events_,
            .mtx = &mtx_};
  }

  auto wait(interval_type interval = interval_type{-1}) -> size_type {
    auto list = lock_exec(std::unique_lock{mtx_}, [&]() {
      return std::vector<event_base>(std::cbegin(interest_),
                                     std::cend(interest_));
    });

    auto num_events =
        poll(list.data(), list.size(), static_cast<int>(interval.count()));
    if (num_events < 0)
      throw_system_error(IO_ERROR_MESSAGE("poll failed."));

    auto end = std::end(list);
    for (auto it = std::begin(list); num_events && it != end; ++it) {
      auto &event = *it;
      if (event.revents && num_events--) {
        auto queues = lock_exec(std::unique_lock{mtx_}, [&]() {
          auto qit = events_.find(event.fd);
          if (qit == std::end(events_))
            return op_queues_{};
          auto tmp = qit->second;
          events_.erase(qit);
          return tmp;
        });

        if (event.revents & POLLIN) {
          auto &queue = queues.read_queue;

          while (!queue.empty()) {
            auto *operation = lock_exec(std::unique_lock{mtx_}, [&]() {
              auto *tmp = queue.front();
              queue.pop();
              return tmp;
            });

            operation->complete();
          }
        }

        if (event.revents & POLLOUT) {
          auto &queue = queues.write_queue;

          while (!queue.empty()) {
            auto *operation = lock_exec(std::unique_lock{mtx_}, [&]() {
              auto *tmp = queue.front();
              queue.pop();
              return tmp;
            });

            operation->complete();
          }
        }
      }
    }

    // TODO: need to think of a way to clean the interest list.

    return num_events;
  }

  // sendmsg
  // template <typename Tag, typename Socket, typename Msg, typename Flags>
  // auto tag_invoke([[maybe_unused]] Tag tag, const Socket &sock, const Msg
  // &msg,
  //                 Flags flags) -> decltype(auto) {
  //   return submit({.fd = sock.fd(), .events = POLLOUT}, [=](event_type event)
  //   {
  //     return ::io::sendmsg(sock, msg, flags);
  //   });
  // }

private:
  interest_list interest_;
  events_type events_;
  mutable std::mutex mtx_;
};

} // namespace io::execution
#endif // IO_POLL_MULTIPLEXER_HPP
