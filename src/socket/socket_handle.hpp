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
 * @brief Defines a cross-platform, thread-safe socket handle with RAII for
 *        robust socket resource management.
 *
 * This file provides the `socket_handle` class, an abstraction over native
 * socket descriptors (file descriptors on POSIX, SOCKET on Windows). It ensures
 * that socket resources are managed safely and automatically using the RAII
 * (Resource Acquisition Is Initialization) idiom.
 *
 * Key features:
 * - **RAII:** Sockets are automatically closed when the `socket_handle` object
 *   goes out of scope, preventing resource leaks.
 * - **Move Semantics:** Ownership is clear and enforced through move-only
 *   semantics (copying is disabled).
 * - **Thread Safety:** Access to the underlying native handle is synchronized,
 *   making it safe for use across multiple threads.
 * - **Cross-Platform:** Provides a uniform interface for both POSIX and Windows
 *   socket APIs.
 */
#pragma once
#ifndef IOSCHED_SOCKET_HANDLE_HPP
#define IOSCHED_SOCKET_HANDLE_HPP
#include "../io.hpp"
#include "socket.hpp"

#include <atomic>
#include <mutex>

/**
 * @namespace io::socket
 * @brief Contains components for cross-platform socket programming.
 */
namespace io::socket {
/**
 * @class socket_handle
 * @brief A thread-safe, move-only RAII wrapper for a native socket handle.
 *
 * This class is the cornerstone of socket lifetime management in this library.
 * It wraps a native socket handle, ensuring it is automatically closed upon
 * destruction. By deleting the copy constructor and copy assignment operator,
 * it enforces unique ownership, preventing common errors related to resource
 * duplication. All access to the handle is protected by a mutex, ensuring
 * thread-safe operations.
 */
class socket_handle {

public:
  /**
   * @brief Default constructor. Initializes an invalid socket handle.
   *
   * A default-constructed handle does not represent a valid socket.
   */
  socket_handle() = default;

  /**
   * @brief Copy constructor (deleted).
   *
   * `socket_handle` is a unique resource-owning type and cannot be copied.
   */
  socket_handle(const socket_handle &other) = delete;

  /**
   * @brief Copy assignment operator (deleted).
   *
   * `socket_handle` is a unique resource-owning type and cannot be copied.
   */
  auto operator=(const socket_handle &other) -> socket_handle & = delete;

  /**
   * @brief Move constructor.
   *
   * Transfers ownership of the socket from another `socket_handle`. After the
   * move, `other` is left in an invalid state.
   *
   * @param other The `socket_handle` to move from.
   */
  socket_handle(socket_handle &&other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * Transfers ownership of the socket from another `socket_handle`. If this
   * handle already owns a socket, it is closed before the new socket is
   * acquired. After the move, `other` is left in an invalid state.
   *
   * @param other The `socket_handle` to move from.
   * @return A reference to this `socket_handle`.
   */
  auto operator=(socket_handle &&other) noexcept -> socket_handle &;

  /**
   * @brief Constructs a `socket_handle` from an existing native socket.
   *
   * Takes ownership of the provided native handle.
   *
   * @param handle The native socket handle to wrap.
   * @throws std::system_error if the handle is not a valid socket.
   */
  explicit socket_handle(native_socket_type handle);

  /**
   * @brief Constructs a `socket_handle` by creating a new socket.
   *
   * @param domain The communication domain (e.g., `AF_INET`).
   * @param type The socket type (e.g., `SOCK_STREAM`).
   * @param protocol The protocol (e.g., `IPPROTO_TCP`).
   * @throws std::system_error if socket creation fails.
   */
  socket_handle(int domain, int type, int protocol);

  /**
   * @brief Explicitly converts the `socket_handle` to its underlying native
   * socket representation.
   *
   * This operation is thread-safe.
   *
   * @return The raw native socket handle.
   */
  explicit operator native_socket_type() const noexcept;

  /**
   * @brief Swaps the contents of two `socket_handle` objects.
   *
   * @param lhs The first `socket_handle`.
   * @param rhs The second `socket_handle`.
   */
  friend auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void;

  /**
   * @brief Checks if the handle owns a valid, open socket.
   *
   * This operation is thread-safe and allows the handle to be used in boolean
   * contexts (e.g., `if (handle)`).
   *
   * @return `true` if the socket handle is valid, `false` otherwise.
   */
  [[nodiscard]] explicit operator bool() const noexcept;

  /**
   * @brief Three-way compares two `socket_handle` objects.
   *
   * The comparison is based on the underlying native socket values. This
   * operation is thread-safe.
   *
   * @param other The `socket_handle` to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto operator<=>(const socket_handle &other) const noexcept
      -> std::strong_ordering;

  /**
   * @brief Checks for equality between two `socket_handle` objects.
   *
   * This operation is thread-safe.
   *
   * @param other The `socket_handle` to compare against.
   * @return `true` if the underlying native socket handles are equal, `false`
   *         otherwise.
   */
  auto operator==(const socket_handle &other) const noexcept -> bool;

  /**
   * @brief Three-way compares the `socket_handle` with a native socket handle.
   *
   * This operation is thread-safe.
   *
   * @param other The native socket handle to compare against.
   * @return A `std::strong_ordering` value.
   */
  auto
  operator<=>(native_socket_type other) const noexcept -> std::strong_ordering;

  /**
   * @brief Checks for equality between a `socket_handle` and a native socket
   * handle.
   *
   * This operation is thread-safe.
   *
   * @param other The native socket handle to compare against.
   * @return `true` if the underlying native socket handles are equal, `false`
   *         otherwise.
   */
  auto operator==(native_socket_type other) const noexcept -> bool;

  /**
   * @brief Destructor.
   *
   * Closes the managed socket if it is valid, ensuring RAII compliance.
   */
  virtual ~socket_handle();

private:
  /**
   * @brief Closes the managed socket if it is valid.
   *
   * This is a helper function called by the destructor and move assignment
   * operator.
   */
  auto close() noexcept -> void;

  /**
   * @brief The underlying native socket handle.
   *
   * It is stored as an atomic type to allow for thread-safe reads of its
   * value, though modifications are protected by the mutex. Initialized to
   * `INVALID_SOCKET` by default.
   */
  std::atomic<native_socket_type> socket_{INVALID_SOCKET};

  /**
   * @brief A mutex to synchronize access to the socket handle for operations
   * that are not inherently atomic (e.g., closing, swapping).
   */
  mutable std::mutex mtx_;
};

/**
 * @brief Binds a socket to a local address.
 * @relates socket_handle
 *
 * Binds the specified socket handle to a local address. This function
 * associates the socket with the address specified by `addr`.
 *
 * @param socket The `socket_handle` to be bound.
 * @param addr A pointer to a `sockaddr_type` structure containing the address
 * to bind to.
 * @param len The length of the `addr` structure.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::bind_t, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int;

} // namespace io::socket
#endif // IOSCHED_SOCKET_HANDLE_HPP
