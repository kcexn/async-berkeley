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
 * @details This multiplexer is used to wait for I/O events on a set of file
 * descriptors. It is used by the `io_context` to dispatch I/O completion
 * handlers.
 */
#pragma once
#include <io/socket/detail/platforms/posix/socket_types.hpp>
#ifndef IO_POLL_MULTIPLEXER_HPP
#define IO_POLL_MULTIPLEXER_HPP
#include "io/execution/detail/execution_trigger.hpp"
#include "multiplexer.hpp"

#include <stdexec/execution.hpp>

#include <map>
#include <memory>

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
 * @details This tag type is used to specialize the `basic_multiplexer`
 * template for a polling multiplexer.
 */
struct poll_t {
  /** @brief The multiplexed event type. */
  using event_type = struct pollfd;
  /** @brief The type used to specify timeouts. */
  using interval_type = std::chrono::milliseconds;
  /** @brief A size type. */
  using size_type = std::size_t;
  /**
   * @brief Type trait to check if an operation should evaluate eagerly.
   * @tparam Op The operation to check.
   */
  template <typename Op> struct is_eager_t : public std::false_type {};
  /**
   * @brief Helper variable template for is_eager_t.
   * @tparam Op The type to check.
   */
  template <typename Op>
  static constexpr bool is_eager_v = is_eager_t<Op>::value;
};

/**
 * @brief A multiplexer that uses the `poll` system call.
 * @details This class is a concrete implementation of the `basic_multiplexer`
 * that uses the `poll` system call to wait for I/O events.
 * @tparam Allocator The allocator to use for all allocations.
 */
template <AllocatorLike Allocator = std::allocator<char>>
class basic_poll_multiplexer : public basic_multiplexer<poll_t> {
public:
  /** @brief The base class for the multiplexer. */
  using Base = basic_multiplexer<poll_t>;
  /** @brief The socket handle type. */
  using socket_handle = ::io::socket::socket_handle;
  /** @brief The task type. */
  using task = Base::intrusive_task_queue::task;
  /** @brief The native socket type. */
  using native_socket_type = ::io::socket::native_socket_type;
  /** @brief The allocator for the vector. */
  using vector_allocator =
      std::allocator_traits<Allocator>::template rebind_alloc<event_type>;
  /** @brief The vector type. */
  using vector_type = std::vector<event_type, vector_allocator>;

  /**
   * @brief Demultiplexes I/O operations for a socket.
   * @details This struct contains the read and write queues for a socket. It is
   * used to queue I/O operations for a socket until they can be completed.
   */
  struct demultiplexer {
    /** @brief Pending read operations. */
    intrusive_task_queue read_queue;
    /** @brief Pending write operations. */
    intrusive_task_queue write_queue;

    /**
     * @brief Associated socket used for setting and getting errors.
     * @note Only valid when the operation state is valid.
     */
    socket_handle *socket = nullptr;
  };

  /** @brief The mapped value. */
  using mapped_value_t =
      typename std::map<native_socket_type, demultiplexer>::value_type;
  /** @brief The allocator for the map. */
  using map_allocator =
      std::allocator_traits<Allocator>::template rebind_alloc<mapped_value_t>;
  /** @brief The map type. */
  using map_type = std::map<native_socket_type, demultiplexer,
                            std::less<native_socket_type>, map_allocator>;

  /**
   * @brief A sender for the poll multiplexer.
   * @details This sender is used to submit I/O operations to the multiplexer.
   * It will complete when the I/O operation is ready.
   * @tparam Fn The function type.
   */
  template <Completion Fn> struct sender {
    /** @brief The sender concept type. */
    using sender_concept = stdexec::sender_t;
    /** @brief The completion signatures for the sender. */
    using completion_signatures = stdexec::completion_signatures<
        stdexec::set_value_t(typename std::invoke_result_t<Fn>::value_type),
        stdexec::set_error_t(int)>;

    /**
     * @brief An operation state for the poll multiplexer.
     * @details This struct contains the state for an I/O operation. It is
     * created when a sender is connected to a receiver.
     * @tparam Receiver The receiver type.
     */
    template <typename Receiver> struct state : public task {
      /**
       * @brief Completes the operation.
       * @param task The task to complete.
       */
      static auto complete(task *task_ptr) noexcept -> void;
      /** @brief Starts the operation. */
      auto start() noexcept -> void;

      /** @brief The socket to operate on. */
      std::shared_ptr<socket_handle> socket;
      /** @brief The completion handler. */
      Fn func;
      /** @brief The demultiplexer for the socket. */
      demultiplexer *demux = nullptr;
      /** @brief A mutex for thread safety. */
      std::mutex *mtx = nullptr;
      /** @brief The receiver to complete. */
      Receiver receiver{};
      /** @brief The poll trigger. */
      execution_trigger trigger{};
    };

    /**
     * @brief Connects the sender to a receiver.
     * @param receiver The receiver to connect to.
     * @return The operation state.
     */
    template <typename Receiver>
    auto connect(Receiver &&receiver) -> state<std::decay_t<Receiver>>;

    /** @brief The socket to operate on. */
    std::shared_ptr<socket_handle> socket;
    /** @brief The completion handler. */
    Fn func;
    /** @brief The poll trigger. */
    execution_trigger trigger{};
    /** @brief The demultiplexer for the socket. */
    demultiplexer *demux = nullptr;
    /** @brief The list of poll events. */
    std::vector<event_type> *list = nullptr;
    /** @brief A mutex for thread safety. */
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
  auto set(std::shared_ptr<socket_handle> socket, execution_trigger trigger,
           Fn &&func) -> sender<std::decay_t<Fn>>;

  /**
   * @brief Default constructor.
   * @param alloc The allocator to use for all allocations.
   */
  constexpr basic_poll_multiplexer(
      const Allocator &alloc = Allocator()) noexcept(noexcept(Allocator()));

private:
  /** @brief A map of file descriptors to demultiplexers. */
  map_type demux_;
  /** @brief A list of poll events. */
  vector_type list_;
  /** @brief A mutex for thread safety. */
  mutable std::mutex mtx_;
};

/**
 * @brief A multiplexer that uses the `poll` system call with the default
 * allocator.
 */
using poll_multiplexer = basic_poll_multiplexer<>;

} // namespace io::execution

#include "io/execution/impl/poll_multiplexer_impl.hpp" // IWYU pragma: export

#endif // IO_POLL_MULTIPLEXER_HPP
