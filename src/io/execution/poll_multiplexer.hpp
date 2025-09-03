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
#include "detail/multiplexer.hpp"

#include <stdexec/execution.hpp>

#include <map>
#include <queue>

#include <poll.h>
// Forward declarations.
namespace io::socket {
class socket_handle;
} // namespace io::socket

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {
/**
 * @brief Tag type for a polling multiplexer.
 */
struct poll_t {
  using event_type = struct pollfd;
  using interval_type = std::chrono::milliseconds;
  using size_type = std::size_t;

  /**
   * @brief Returns the key for a multiplexed event.
   * @param event The multiplexed event of event_type.
   * @return The key for the event.
   */
  [[nodiscard]] static constexpr auto
  key(const event_type &event) noexcept -> int {
    return event.fd;
  }
};

/**
 * @brief A multiplexer that uses the `poll` system call.
 */
struct poll_multiplexer : public basic_multiplexer<poll_t> {
  using Base = basic_multiplexer<poll_t>;
  using socket_handle = ::io::socket::socket_handle;

  /**
   * @brief Demultiplexes I/O operations for a socket.
   */
  struct demultiplexer {
    /**
     * @brief Pending read operations.
     */
    std::queue<task *> read_queue;
    /**
     * @brief Pending write operations.
     */
    std::queue<task *> write_queue;
    /**
     * @brief Associated socket used for setting and getting errors.
     * @note Only valid when the operation state is valid.
     */
    socket_handle *socket;
  };

  /**
   * @brief A sender for the poll multiplexer.
   * @tparam Fn The function type.
   */
  template <Completion Fn> struct sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::streamsize),
                                       stdexec::set_error_t(int)>;

    /**
     * @brief An operation state for the poll multiplexer.
     * @tparam Receiver The receiver type.
     */
    template <typename Receiver> struct state : public task {
      /**
       * @brief Completes the operation.
       * @param task The task to complete.
       */
      static auto complete(task *task_ptr) -> void;
      /**
       * @brief Starts the operation.
       */
      auto start() noexcept -> void;

      /**
       * @brief The socket to operate on.
       */
      std::shared_ptr<socket_handle> socket;
      /**
       * @brief The completion handler.
       */
      Fn func{};
      /**
       * @brief The demultiplexer for the socket.
       */
      demultiplexer *demux = nullptr;
      /**
       * @brief The mutex for thread safety.
       */
      std::mutex *mtx = nullptr;
      /**
       * @brief The receiver to complete.
       */
      Receiver receiver{};
      /**
       * @brief The event mask to wait for.
       */
      short mask = 0;
    };

    /**
     * @brief Connects the sender to a receiver.
     * @param receiver The receiver to connect to.
     * @return The operation state.
     */
    template <typename Receiver>
    auto connect(Receiver &&receiver) -> state<std::decay_t<Receiver>>;

    /**
     * @brief The socket to operate on.
     */
    std::shared_ptr<socket_handle> socket;
    /**
     * @brief The completion handler.
     */
    Fn func{};
    /**
     * @brief The event to wait for.
     */
    event_type event{};
    /**
     * @brief The demultiplexer for the socket.
     */
    demultiplexer *demux = nullptr;
    /**
     * @brief The list of events to poll.
     */
    std::vector<event_type> *list = nullptr;
    /**
     * @brief The mutex for thread safety.
     */
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
   * @param socket The socket to set the completion handler for.
   * @param event The event to wait for.
   * @param func The completion handler.
   * @return A sender that will complete when the event occurs.
   */
  template <Completion Fn>
  auto set(std::shared_ptr<socket_handle> socket, event_type event,
           Fn &&func) -> sender<std::decay_t<Fn>>;

private:
  /**
   * @brief Map of file descriptors to demultiplexers.
   */
  std::map<int, demultiplexer> demux_;
  /**
   * @brief List of events to poll.
   */
  std::vector<event_type> list_;
  /**
   * @brief Mutex for thread safety.
   */
  mutable std::mutex mtx_;
};

} // namespace io::execution

#include "impl/poll_multiplexer_impl.hpp" // IWYU pragma: export

#endif // IO_POLL_MULTIPLEXER_HPP
