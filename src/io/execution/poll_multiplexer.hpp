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

/**
 * @file poll_multiplexer.hpp
 * @brief This file defines a multiplexer that uses the `poll` system call.
 */
#pragma once
#ifndef IO_POLL_MULTIPLEXER_HPP
#define IO_POLL_MULTIPLEXER_HPP
#include "detail/execution_concepts.hpp"
#include "detail/immovable.hpp"
#include <io/error.hpp>
#include <io/socket/socket_handle.hpp>

#include <stdexec/execution.hpp>

#include <chrono>
#include <ios>
#include <list>
#include <mutex>
#include <queue>

#include <poll.h>

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief A task for the poll multiplexer.
 */
struct poll_task {
  void (*do_complete)(poll_task *) = nullptr;
};

/**
 * @brief An event for the poll multiplexer.
 */
struct poll_event {
  /**
   * @brief Gets the key for the event.
   * @return The key for the event.
   */
  [[nodiscard]] constexpr auto key() const -> int { return pfd.fd; }

  struct pollfd pfd {};
  std::queue<poll_task *> read_queue, write_queue;
};

/**
 * @brief A completion handler for the poll multiplexer.
 */
struct poll_completion {
  /**
   * @brief Executes the completion handler.
   */
  auto operator()() const noexcept -> void;

  short revents = 0;
  poll_event *event = nullptr;
  std::mutex *mtx = nullptr;
};

/**
 * @brief A multiplexer that uses the `poll` system call.
 */
class poll_multiplexer {

public:
  // type definitions
  using size_type = std::size_t;
  static constexpr size_type MUX_ERROR = std::numeric_limits<size_type>::max();
  using interval_type = std::chrono::milliseconds;
  using event_type = struct pollfd;

  /**
   * @brief An operation state for the poll multiplexer.
   * @tparam Receiver The receiver type.
   * @tparam Fn The function type.
   */
  template <typename Receiver, Completion Fn>
  struct poll_op : public immovable, public poll_task {

    /**
     * @brief Completes the operation.
     * @param task The task to complete.
     */
    static auto complete(poll_task *task) noexcept -> void;
    /**
     * @brief Starts the operation.
     */
    auto start() noexcept -> void;

    Receiver receiver{};
    Fn func{};
    short events_mask = 0;
    poll_event *event = nullptr;
    std::mutex *mtx = nullptr;
  };

  /**
   * @brief A sender for the poll multiplexer.
   * @tparam Fn The function type.
   */
  template <Completion Fn> struct poll_sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    /**
     * @brief Connects the sender to a receiver.
     * @param receiver The receiver to connect to.
     * @return The operation state.
     */
    template <typename Receiver>
    auto connect(Receiver receiver) -> poll_op<Receiver, Fn>;

    Fn func{};
    poll_event event{};
    std::list<poll_event> *list = nullptr;
    std::mutex *mtx = nullptr;
  };

  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return The number of events that occurred.
   */
  auto wait_for(interval_type interval) -> size_type;

  /**
   * @brief Sets a completion handler for an event.
   * @tparam Fn The function type.
   * @param event The event to wait for.
   * @param func The completion handler.
   * @return A sender that will complete when the event occurs.
   */
  template <Completion Fn>
  auto set(event_type event, Fn func) -> poll_sender<Fn>;

private:
  std::list<poll_event> list_;
  mutable std::mutex mtx_;
};

} // namespace io::execution

#include "impl/poll_multiplexer_impl.hpp" // IWYU pragma: export

#endif // IO_POLL_MULTIPLEXER_HPP
