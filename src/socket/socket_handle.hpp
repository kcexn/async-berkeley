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
 * @file socket/socket_handle.hpp
 * @brief Defines a cross-platform, thread-safe socket handle with RAII.
 *
 * This file provides a `socket_handle` class that abstracts away the
 * differences between Windows (SOCKET) and POSIX (file descriptor) socket
 * implementations. It uses conditional compilation to select the appropriate
 * native socket types and functions, allowing for a single, unified interface.
 *
 * Key features include:
 * - RAII-based management to ensure sockets are automatically closed.
 * - Move-only semantics to enforce clear ownership.
 * - Thread-safe access to the underlying native socket handle.
 * - A consistent error-handling model using `std::system_error`.
 *
 * The primary component is:
 * - `socket_handle`: A thread-safe, cross-platform socket handle.
 */
#pragma once
#ifndef IOSCHED_SOCKET_HANDLE_HPP
#define IOSCHED_SOCKET_HANDLE_HPP
#include "socket.hpp"

#include <compare>
#include <mutex>

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace iosched::socket {
/**
 * @class socket_handle
 * @brief A thread-safe, move-only RAII wrapper for a native socket handle.
 *
 * This class provides the core functionality for socket lifetime management. It
 * ensures that a valid socket is automatically closed when it goes out of
 * scope. To prevent resource mismanagement, it is move-only and cannot be
 * copied. Access to the underlying handle is synchronized via a mutex, making
 * it safe for use in multi-threaded contexts.
 */
class socket_handle {

public:
  /**
   * @brief Default constructor.
   *
   * Initializes an empty handle that does not own a socket.
   */
  socket_handle() = default;

  socket_handle(const socket_handle &other) = delete;
  auto operator=(const socket_handle &other) -> socket_handle & = delete;

  /**
   * @brief Move constructor.
   *
   * Takes ownership of the socket from another `socket_handle`.
   * @param other The `socket_handle` to move from.
   */
  socket_handle(socket_handle &&other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * Takes ownership of the socket from another `socket_handle`.
   * @param other The `socket_handle` to move from.
   * @return A reference to this `socket_handle`.
   */
  auto operator=(socket_handle &&other) noexcept -> socket_handle &;

  /**
   * @brief Constructs a `socket_handle` from a native socket.
   *
   * Takes ownership of an existing native socket handle.
   * @param handle The native socket handle to manage.
   */
  explicit socket_handle(native_socket_type handle) noexcept
      : socket_{handle} {}

  /**
   * @brief Creates a new socket and wraps it in a `socket_handle`.
   *
   * @param domain The communication domain (e.g., `AF_INET`).
   * @param type The socket type (e.g., `SOCK_STREAM`).
   * @param protocol The protocol (e.g., `IPPROTO_TCP`).
   * @throws std::system_error if socket creation fails.
   */
  explicit socket_handle(int domain, int type, int protocol);

  /**
   * @brief Destructor.
   *
   * Closes the managed socket if it is valid.
   */
  ~socket_handle();

  /**
   * @brief Gets the underlying native socket handle.
   *
   * This operation is thread-safe.
   * @return The native socket handle.
   */
  [[nodiscard]] explicit operator native_socket_type() const noexcept;

  /**
   * @brief Swaps the contents of two `socket_handle` objects.
   *
   * @param lhs The first `socket_handle`.
   * @param rhs The second `socket_handle`.
   */
  friend auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void;

  /**
   * @brief Checks if the handle owns a valid socket.
   *
   * This operation is thread-safe.
   * @return `true` if the socket handle is valid, `false` otherwise.
   */
  [[nodiscard]] explicit operator bool() const noexcept;

  /**
   * @brief Compares two `socket_handle` objects for ordering.
   *
   * This operation is thread-safe.
   * @param other The `socket_handle` to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto operator<=>(const socket_handle &other) const noexcept
      -> std::strong_ordering;

  /**
   * @brief Compares two `socket_handle` objects for inequality.
   *
   * This operation is thread-safe.
   * @param other The `socket_handle` to compare against.
   * @return `true` if the socket handles are not equal, `false` otherwise.
   */
  auto operator!=(const socket_handle &other) const noexcept -> bool;

private:
  /**
   * @brief Closes the managed socket if it is valid.
   */
  auto close() noexcept -> void;

  /**
   * @brief The native socket handle managed by this object.
   */
  native_socket_type socket_{INVALID_SOCKET};
  /**
   * @brief A mutex to synchronize access to the socket handle.
   */
  mutable std::mutex mtx_;
};

} // namespace iosched::socket
#endif // IOSCHED_SOCKET_HANDLE_HPP
