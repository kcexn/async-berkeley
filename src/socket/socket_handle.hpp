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
#include <compare>
#ifndef IOSCHED_SOCKET_HANDLE_HPP
#define IOSCHED_SOCKET_HANDLE_HPP
#include "socket.hpp"

#include <atomic>
#include <mutex>

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace iosched::socket {

/**
 * @struct socket_handle_state
 * @brief Holds the state for a socket handle.
 *
 * This struct contains the native socket handle and provides constructors
 * for initializing it. It is used as a base class for `socket_handle` to
 * separate state from the RAII and locking logic.
 */
struct socket_handle_state {
  /**
   * @brief The native socket handle.
   *
   * Initialized to `INVALID_SOCKET` by default.
   */
  std::atomic<native_socket_type> socket{INVALID_SOCKET};

  /// @brief Default constructor. Initializes the socket to an invalid state.
  socket_handle_state() = default;
  /**
   * @brief Copy constructor.
   * @param other The object to copy from.
   */
  socket_handle_state(const socket_handle_state &other);
  /**
   * @brief Copy assignment operator.
   * @param other The object to copy from.
   * @return A reference to this object.
   */
  auto operator=(const socket_handle_state &other) -> socket_handle_state &;
  /**
   * @brief Move constructor.
   * @param other The object to move from.
   */
  socket_handle_state(socket_handle_state &&other) noexcept;
  /**
   * @brief Move assignment operator.
   * @param other The object to move from.
   * @return A reference to this object.
   */
  auto operator=(socket_handle_state &&other) noexcept -> socket_handle_state &;

  /**
   * @brief Constructs a state object from an existing native socket handle.
   * @param handle The native socket handle to store.
   */
  explicit socket_handle_state(native_socket_type handle) noexcept;
  /**
   * @brief Constructs a state object by creating a new socket.
   * @param domain The communication domain (e.g., `AF_INET`).
   * @param type The socket type (e.g., `SOCK_STREAM`).
   * @param protocol The protocol (e.g., `IPPROTO_TCP`).
   * @throws std::system_error if socket creation fails.
   */
  socket_handle_state(int domain, int type, int protocol);

  /**
   * @brief Swaps the contents of two `socket_handle_state` objects.
   * @param lhs The first object.
   * @param rhs The second object.
   */
  friend auto swap(socket_handle_state &lhs,
                   socket_handle_state &rhs) noexcept -> void;

  /**
   * @brief Checks if the handle owns a valid socket.
   *
   * This operation is thread-safe.
   * @return `true` if the socket handle is valid, `false` otherwise.
   */
  [[nodiscard]] virtual explicit operator bool() const noexcept;

  /**
   * @brief Compares two `socket_handle_state` objects for ordering.
   *
   * This operation is thread-safe.
   * @param other The `socket_handle_state` to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto operator<=>(const socket_handle_state &other) const noexcept
      -> std::strong_ordering;

  /**
   * @brief Compares two `socket_handle_state` objects for equality.
   *
   * This operation is thread-safe.
   * @param other The `socket_handle_state` to compare against.
   * @return `true` if the socket handles are equal, `false` otherwise.
   */
  auto operator==(const socket_handle_state &other) const noexcept -> bool;

  /**
   * @brief Compares the `socket_handle` with a native socket handle for
   * ordering.
   *
   * This operation is thread-safe.
   * @param other The native socket handle to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto
  operator<=>(native_socket_type other) const noexcept -> std::strong_ordering;

  /**
   * @brief Compares the `socket_handle` with a native socket handle for
   * equality.
   *
   * This operation is thread-safe.
   * @param other The native socket handle to compare against.
   * @return `true` if the socket handles are equal, `false` otherwise.
   */
  auto operator==(native_socket_type other) const noexcept -> bool;

  /**
   * @brief Virtual destructor.
   *
   * Ensures that derived classes are properly destroyed.
   */
  virtual ~socket_handle_state() = default;
};

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
class socket_handle : public socket_handle_state {

public:
  using socket_handle_state::socket_handle_state;
  using socket_handle_state::operator<=>;
  using socket_handle_state::operator==;
  using Base = socket_handle_state;

  /// @brief Default constructor. Initializes an invalid socket handle.
  socket_handle() = default;

  /**
   * @brief Copy constructor is deleted.
   *
   * `socket_handle` is a unique resource-owning type and cannot be copied.
   */
  socket_handle(const socket_handle &other) = delete;
  /**
   * @brief Copy assignment operator is deleted.
   *
   * `socket_handle` is a unique resource-owning type and cannot be copied.
   */
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
   * @brief Swaps the contents of two `socket_handle` objects.
   *
   * @param lhs The first `socket_handle`.
   * @param rhs The second `socket_handle`.
   */
  friend auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void;

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
   * @brief Compares two `socket_handle` objects for equality.
   *
   * This operation is thread-safe.
   * @param other The `socket_handle` to compare against.
   * @return `true` if the socket handles are equal, `false` otherwise.
   */
  auto operator==(const socket_handle &other) const noexcept -> bool;

  /**
   * @brief Destructor.
   *
   * Closes the managed socket if it is valid.
   */
  virtual ~socket_handle();

private:
  /**
   * @brief Closes the managed socket if it is valid.
   */
  auto close() noexcept -> void;

  /**
   * @brief A mutex to synchronize access to the socket handle.
   */
  mutable std::mutex mtx_;
};

} // namespace iosched::socket
#endif // IOSCHED_SOCKET_HANDLE_HPP
