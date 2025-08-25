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
#include <error.hpp>
#include <io.hpp>
#include <socket/socket_handle.hpp>

#include <stdexec/execution.hpp>

#include <chrono>
#include <ios>
#include <list>
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

  template <typename Receiver, typename Fn>
    requires std::is_invocable_v<Fn, event_type *>
  struct poll_op : public detail::immovable, public poll_task {

    static auto complete(poll_task *task) noexcept -> void;
    auto start() noexcept -> void;

    Receiver receiver{};
    Fn func{};
    short events_mask = 0;
    poll_event *event = nullptr;
    std::mutex *mtx = nullptr;
  };

  template <typename Fn>
    requires std::is_invocable_v<Fn, event_type *>
  struct poll_sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    template <typename Receiver>
    auto connect(Receiver receiver) -> poll_op<Receiver, Fn>;

    Fn func{};
    poll_event event{};
    std::list<poll_event> *list = nullptr;
    std::mutex *mtx = nullptr;
  };

  auto run_once_for(interval_type interval = interval_type{-1}) -> size_type;

  template <typename Fn>
    requires std::is_invocable_v<Fn, event_type *>
  auto submit(event_type event, Fn func) -> poll_sender<Fn>;

private:
  std::list<poll_event> list_;
  mutable std::mutex mtx_;
};

} // namespace io::execution

#include "impl/poll_multiplexer_impl.hpp" // IWYU pragma: export

#endif // IO_POLL_MULTIPLEXER_HPP
