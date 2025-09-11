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
#include "detail/execution_trigger.hpp"
#include "io/detail/concepts.hpp"
#include "io/detail/customization.hpp"
#include "io/error.hpp"
#include "io/socket/socket.hpp"

#include <exec/async_scope.hpp>
#include <stdexec/execution.hpp>

#include <utility>
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
   * @brief The size type used by the multiplexer.
   */
  using size_type = Mux::size_type;
  /**
   * @internal
   * @brief The interval type used by the multiplexer for timeouts.
   */
  using interval_type = Mux::interval_type;

public:
  /**
   * @brief The type of socket handle used by the executor.
   */
  using socket_handle = ::io::socket::socket_handle;

  /**
   * @brief Pushes a socket handle to the collection.
   * @param handle The socket handle to push.
   * @return A weak pointer to the pushed socket handle.
   */
  static auto push(socket_handle &&handle) -> std::shared_ptr<socket_handle> {
    if (::io::fcntl(handle, F_SETFL, ::io::fcntl(handle, F_GETFL) | O_NONBLOCK))
      throw_system_error("fcntl failed.");
    return std::make_shared<socket_handle>(std::move(handle));
  }

  /**
   * @brief Emplaces a socket handle in the collection.
   * @param ...args The arguments to forward to the socket handle constructor.
   * @return A shared pointer to the emplaced socket handle.
   */
  template <typename... Args>
  static auto emplace(Args &&...args) -> std::shared_ptr<socket_handle> {
    auto ptr = std::make_shared<socket_handle>(std::forward<Args>(args)...);
    if (::io::fcntl(*ptr, F_SETFL, ::io::fcntl(*ptr, F_GETFL) | O_NONBLOCK))
      throw_system_error("fcntl failed.");
    return std::move(ptr);
  }

  /**
   * @brief Sets a completion handler for an event.
   * @param event The event to wait for.
   * @param exec The completion handler.
   * @return A future that will complete when the event occurs.
   */
  template <Completion Fn>
  auto set(std::shared_ptr<::io::socket::socket_handle> socket,
           execution_trigger event, Fn &&exec) -> decltype(auto) {
    return scope_.spawn_future(
        Mux::set(std::move(socket), event, std::forward<Fn>(exec)));
  }

  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return The number of events that occurred.
   */
  constexpr auto wait_for(int interval = -1) -> decltype(auto) {
    return Mux::wait_for(interval_type{interval});
  }

  /**
   * @brief Waits for events to occur.
   * @return The number of events that occurred.
   */
  constexpr auto wait() -> decltype(auto) { return wait_for(); }

private:
  /**
   * @brief The async scope for the executor.
   */
  exec::async_scope scope_;
};

} // namespace io::execution
#endif // IO_EXECUTOR_HPP
