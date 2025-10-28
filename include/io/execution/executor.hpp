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
 * @file executor.hpp
 * @brief This file defines a generic executor for the I/O library.
 */
#pragma once
#ifndef IO_EXECUTOR_HPP
#define IO_EXECUTOR_HPP
#include "io/detail/concepts.hpp"
#include "io/detail/customization.hpp"
#include "io/error.hpp"
#include "io/socket/socket_handle.hpp"

#include <exec/async_scope.hpp>
#include <stdexec/execution.hpp>

#include <utility>
// Forward declarations
namespace io::execution {
template <Multiplexer Mux> class basic_triggers;
} // namespace io::execution

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {
/**
 * @brief An executor that uses a multiplexer to wait for events.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> class executor : public Mux {
  /**
   * @internal
   * @brief Grants basic_triggers access to the private members of this class.
   */
  friend class basic_triggers<Mux>;
  /**
   * @internal
   * @brief The type of socket handle used by the executor.
   */
  using socket_handle = ::io::socket::socket_handle;
  /**
   * @internal
   * @brief The type of async_scope
   */
  using async_scope = exec::async_scope;

public:
  /** @brief Use the base class constructor. */
  using Mux::Mux;
  /**
   * @brief Configures the socket to be non-blocking.
   *
   * This overload can be used to create socket_dialogs
   * using a custom allocator for the socket handle.
   *
   * @tparam Socket A socket handle like object.
   * @param socket A shared pointer to a non-blocking socket.
   * @return A shared pointer to a non-blocking socket.
   */
  template <SocketLike Socket>
  static auto push(std::shared_ptr<Socket> socket) -> decltype(auto)
  {
    if (::io::fcntl(*socket, F_SETFL,
                    ::io::fcntl(*socket, F_GETFL) | O_NONBLOCK))
    {
      throw_system_error(IO_ERROR_MESSAGE("fcntl failed."));
    }
    return socket;
  }
  /**
   * @brief Pushes a socket handle to the collection.
   * @param handle The socket handle to push.
   * @return A weak pointer to the pushed socket handle.
   */
  template <SocketLike Socket>
  static auto push(Socket &&handle) -> decltype(auto)
  {
    return push(std::make_shared<Socket>(std::forward<Socket>(handle)));
  }
  /**
   * @brief Emplaces a socket handle in the collection.
   * @param ...args The arguments to forward to the socket handle constructor.
   * @return A shared pointer to the emplaced socket handle.
   */
  template <typename... Args>
  static auto emplace(Args &&...args) -> std::shared_ptr<socket_handle>
  {
    return push(socket_handle{std::forward<Args>(args)...});
  }
  /**
   * @brief Sets a completion handler for an event.
   * @param args The arguments to forward to the multiplexer.
   * @tparam Args The types of the perfectly forwarded arguments.
   * @return A sender, scoped to the lifetime of the executor.
   */
  template <typename... Args> auto set(Args &&...args) -> decltype(auto)
  {
    return scope_.nest(Mux::set(std::forward<Args>(args)...));
  }
  /**
   * @brief Sends a notice when the executor is empty.
   * @returns A sender that notifies when the executor is empty.
   */
  [[nodiscard]] auto on_empty() -> decltype(auto) { return scope_.on_empty(); }

private:
  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return The number of events that occurred.
   */
  constexpr auto wait_for(int interval = -1) -> decltype(auto)
  {
    return Mux::wait_for(typename Mux::interval_type{interval});
  }
  /**
   * @brief Waits for events to occur.
   * @return The number of events that occurred.
   */
  constexpr auto wait() -> decltype(auto) { return wait_for(); }
  /** @brief The async scope for the executor. */
  async_scope scope_;
};

} // namespace io::execution
#endif // IO_EXECUTOR_HPP
