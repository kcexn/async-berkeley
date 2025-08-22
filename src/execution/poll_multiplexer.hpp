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

#include <chrono>
#include <ios>
#include <list>
#include <map>
#include <mutex>
#include <queue>

#include <poll.h>

namespace io::execution {
class poll_multiplexer {

public:
  // type definitions
  using size_type = std::size_t;
  static constexpr size_type MUX_ERROR = std::numeric_limits<size_type>::max();
  using interval_type = std::chrono::milliseconds;
  using event_base = struct pollfd;
  struct event_type : public event_base {
    [[nodiscard]] constexpr auto key() const -> int { return fd; }
  };
  using interest_list = std::list<event_type>;

  struct operation : detail::immovable {
    operation() = default;
    operation(const operation &) = default;
    auto operator=(const operation &) -> operation & = default;
    operation(operation &&) = delete;
    auto operator=(operation &&) -> operation & = delete;

    event_type *event{};
    interest_list *interest{};
    std::mutex *mtx{};

    virtual auto complete() -> void = 0;

    virtual ~operation() = default;
  };

  struct op_queues {
    std::queue<operation *> read_queue;
    std::queue<operation *> write_queue;
  };

  using events_type = std::map<decltype(event_type{}.key()), op_queues>;

  template <typename Receiver, detail::Operation<event_type> Fn>
  struct poll_op : public operation {
    Receiver receiver;
    events_type *events{};
    Fn &&func;

    auto complete() -> void override;
    auto start() -> void;
  };

  template <detail::Operation<event_type> Fn> struct poll_sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    template <typename Receiver>
    auto connect(Receiver receiver) -> poll_op<Receiver, Fn>;

    Fn func;
    event_type event;
    interest_list *interest;
    events_type *events;
    std::mutex *mtx;
  };

  template <detail::Operation<event_type> Fn>
  auto submit(event_type event, Fn func) -> poll_sender<Fn>;
  auto run_for(interval_type interval = interval_type{-1}) -> size_type;

private:
  interest_list interest_;
  events_type events_;
  mutable std::mutex mtx_;
};

} // namespace io::execution

#include "impl/poll_multiplexer_impl.hpp" // IWYU pragma: export

#endif // IO_POLL_MULTIPLEXER_HPP
