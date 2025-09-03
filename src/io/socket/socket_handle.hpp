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
 * @file socket_handle.hpp
 * @brief Defines the `socket_handle` class, a cross-platform, thread-safe RAII
 * wrapper for native socket handles.
 */
#pragma once
#ifndef IO_SOCKET_HANDLE_HPP
#define IO_SOCKET_HANDLE_HPP
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include "platforms/windows/socket.hpp"
#else
#include "platforms/posix/socket.hpp"
#endif

#include <atomic>
#include <mutex>

namespace io::socket {
/**
 * @brief A thread-safe, move-only RAII wrapper for a native socket handle.
 *
 * Wraps a native socket handle, ensuring it is automatically closed upon
 * destruction. Enforces unique ownership by deleting copy operations. All
 * access to the handle is thread-safe.
 */
class socket_handle {

public:
  /**
   * @brief Initializes an invalid socket handle.
   */
  socket_handle() = default;

  /**
   * @brief Deleted copy constructor. A `socket_handle` cannot be copied.
   */
  socket_handle(const socket_handle &other) = delete;

  /**
   * @brief Deleted copy assignment. A `socket_handle` cannot be copied.
   */
  auto operator=(const socket_handle &other) -> socket_handle & = delete;

  /**
   * @brief Move constructor. Transfers ownership of a socket from `other`.
   * `other` becomes invalid.
   * @param other The `socket_handle` to move from.
   */
  socket_handle(socket_handle &&other) noexcept;

  /**
   * @brief Move assignment. Transfers ownership of a socket from `other`. The
   * existing socket is closed. `other` becomes invalid.
   * @param other The `socket_handle` to move from.
   * @return A reference to this `socket_handle`.
   */
  auto operator=(socket_handle &&other) noexcept -> socket_handle &;

  /**
   * @brief Constructs from a native socket handle.
   * @param handle The native socket handle to wrap.
   * @throws std::system_error if the handle is not a valid socket.
   */
  explicit socket_handle(native_socket_type handle);

  /**
   * @brief Constructs a new socket.
   * @param domain The communication domain (e.g., `AF_INET`).
   * @param type The socket type (e.g., `SOCK_STREAM`).
   * @param protocol The protocol (e.g., `IPPROTO_TCP`).
   * @throws std::system_error if socket creation fails.
   */
  socket_handle(int domain, int type, int protocol);

  /**
   * @brief Gets the underlying native socket handle.
   * @return The raw native socket handle.
   */
  explicit operator native_socket_type() const noexcept;

  /**
   * @brief Swaps two `socket_handle` objects.
   * @param lhs The first `socket_handle`.
   * @param rhs The second `socket_handle`.
   */
  friend auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void;

  /**
   * @brief Checks if the socket handle is valid.
   * @return `true` if the socket handle is valid, `false` otherwise.
   */
  [[nodiscard]] explicit operator bool() const noexcept;

  /**
   * @brief Compares two `socket_handle` objects.
   * @param other The `socket_handle` to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto operator<=>(const socket_handle &other) const noexcept
      -> std::strong_ordering;

  /**
   * @brief Checks for equality between two `socket_handle` objects.
   * @param other The `socket_handle` to compare against.
   * @return `true` if the underlying native socket handles are equal, `false`
   * otherwise.
   */
  auto operator==(const socket_handle &other) const noexcept -> bool;

  /**
   * @brief Compares with a native socket handle.
   * @param other The native socket handle to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto
  operator<=>(native_socket_type other) const noexcept -> std::strong_ordering;

  /**
   * @brief Checks for equality with a native socket handle.
   * @param other The native socket handle to compare against.
   * @return `true` if the underlying native socket handles are equal, `false`
   * otherwise.
   */
  auto operator==(native_socket_type other) const noexcept -> bool;

  /**
   * @brief Sets a socket error.
   * @param error The error code to set.
   */
  auto set_error(int error) noexcept -> void;

  /**
   * @brief Gets the last socket error.
   * @return The last error code.
   */
  auto get_error() const noexcept -> std::error_code;

  /**
   * @brief Closes the managed socket.
   */
  ~socket_handle();

private:
  /**
   * @brief Closes the managed socket.
   */
  auto close() noexcept -> void;

  /**
   * @brief The underlying native socket handle.
   */
  std::atomic<native_socket_type> socket_{INVALID_SOCKET};
  /**
   * @brief The last error code on the socket.
   */
  std::atomic<int> error_;
  /**
   * @brief The mutex for thread safety.
   */
  mutable std::mutex mtx_;
};

} // namespace io::socket
#endif // IO_SOCKET_HANDLE_HPP
