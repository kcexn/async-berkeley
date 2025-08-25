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
#include "detail/immovable.hpp"
#include "detail/lock_exec.hpp"
#include <algorithm>
#include <error.hpp>
#include <io.hpp>
#include <socket/socket_handle.hpp>

#include <stdexec/execution.hpp>

#include <chrono>
#include <ios>
#include <list>
#include <map>
#include <mutex>
#include <queue>

#include <poll.h>

namespace io::execution {

struct poll_task {
  void (*do_complete)(poll_task *) = nullptr;
};

struct poll_event {
  [[nodiscard]] constexpr auto key() const -> int { return pfd.fd; }

  struct pollfd pfd {};
  std::queue<poll_task *> read_queue, write_queue;
};

struct poll_completion {
  auto operator()() const noexcept -> void;

  short revents = 0;
  poll_event *event = nullptr;
  std::mutex *mtx = nullptr;
};

class poll_multiplexer {

public:
  // type definitions
  using size_type = std::size_t;
  static constexpr size_type MUX_ERROR = std::numeric_limits<size_type>::max();
  using interval_type = std::chrono::milliseconds;
  using event_type = struct pollfd;
  using interest_list = std::list<poll_event>;

  template <typename Receiver, typename Fn>
    requires std::is_invocable_v<Fn, event_type *>
  struct poll_op : public detail::immovable, public poll_task {
    static auto complete(poll_task *ptr) noexcept -> void {
      auto *self = static_cast<poll_op *>(ptr);
      std::streamsize len = self->func(self->event);
      if (len >= 0) {
        stdexec::set_value(std::move(self->receiver), len);
      } else {
        stdexec::set_error(std::move(self->receiver), errno);
      }
    }

    auto start() noexcept -> void {
      std::lock_guard lock{*mtx};

      poll_task::do_complete = poll_op::complete;
      event->pfd.events |= events_mask;

      if (events_mask & POLLIN)
        event->read_queue.push(this);

      if (events_mask & POLLOUT)
        event->write_queue.push(this);
    }

    Receiver receiver{};
    Fn func{};
    short events_mask = 0;
    poll_event *event = nullptr;
    std::mutex *mtx = nullptr;
  };

  template <typename Fn>
    requires std::is_invocable_v<Fn, struct pollfd *>
  struct poll_sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    static auto
    update_or_insert_event(interest_list *list,
                           const poll_event &event) -> decltype(auto) {
      auto event_it = std::ranges::lower_bound(
          *list, event.key(), {},
          [](const auto &event) { return event.pfd.fd; });

      if (event_it != std::end(*list) && event_it->pfd.fd == event.key()) {
        event_it->pfd.events |= event.pfd.events;
      } else {
        event_it = list->insert(event_it, event);
      }

      return event_it;
    }

    template <typename Receiver>
    auto connect(Receiver receiver) -> poll_op<Receiver, Fn> {
      using detail::lock_exec;

      auto event_it = lock_exec(std::unique_lock{*mtx}, [&]() {
        return update_or_insert_event(list, event);
      });

      return {.receiver = std::move(receiver),
              .func = std::move(func),
              .events_mask = event.pfd.events,
              .event = &(*event_it),
              .mtx = mtx};
    }

    Fn func{};
    poll_event event{};
    interest_list *list = nullptr;
    std::mutex *mtx = nullptr;
  };

  template <typename Fn>
    requires std::is_invocable_v<Fn, event_type *>
  auto submit(event_type event, Fn func) -> poll_sender<Fn> {
    return {.func = std::move(func),
            .event = {.pfd = std::move(event)},
            .list = &list_,
            .mtx = &mtx_};
  }

  auto run_once_for(interval_type interval = interval_type{-1}) -> size_type;

private:
  std::list<poll_event> list_;
  mutable std::mutex mtx_;
};

} // namespace io::execution

#endif // IO_POLL_MULTIPLEXER_HPP
