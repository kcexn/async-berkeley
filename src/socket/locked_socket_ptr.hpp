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
 * @file locked_socket_ptr.hpp
 * @brief Defines a RAII-based smart pointer for locked socket access.
 *
 * This file provides the `locked_socket_ptr` class, a smart-pointer-like
 * type that provides temporary, thread-safe access to a native socket handle.
 * It is designed to be returned by a factory method (like
 * `socket_handle::get()`) and holds a `std::unique_lock` for its entire
 * lifetime.
 *
 * This ensures that any operations performed on the underlying socket via this
 * pointer are synchronized, preventing data races in multi-threaded contexts.
 *
 * Key features:
 * - RAII-based lock management.
 * - Pointer-like interface (`operator*`, `operator->`, `get`).
 * - Non-copyable and non-movable to enforce strict scoped locking.
 *
 * The primary component is:
 * - `locked_socket_ptr`: A smart pointer that holds a lock on a socket.
 */
#pragma once
#ifndef IOSCHED_LOCKED_SOCKET_PTR_HPP
#define IOSCHED_LOCKED_SOCKET_PTR_HPP
#include "socket.hpp"
#include <mutex>

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace iosched::socket {
/**
 * @class locked_socket_ptr
 * @brief A RAII smart pointer that provides locked access to a native socket.
 *
 * This class holds a `std::unique_lock` on a mutex, providing exclusive,
 * synchronized access to a native socket handle for the lifetime of the
 * `locked_socket_ptr` object. It exposes a smart-pointer-like interface
 * (`operator*`, and `get()`) for interacting with the socket.
 *
 * It is intended to be created and returned by a resource-owning class
 * (e.g., `socket_handle`) to grant temporary, safe access to its internal
 * socket.
 *
 * @note This class is non-copyable and non-movable to ensure that the lock's
 *       lifetime is strictly tied to the scope in which it was created.
 */
class locked_socket_ptr {
public:
  locked_socket_ptr(const locked_socket_ptr &) = delete;
  auto operator=(const locked_socket_ptr &) -> locked_socket_ptr & = delete;
  locked_socket_ptr(locked_socket_ptr &&) = delete;
  auto operator=(locked_socket_ptr &&) -> locked_socket_ptr & = delete;

  /**
   * @brief Constructs a locked pointer with the given lock and socket
   * reference.
   * @param lock The lock guard to hold during the lifetime of this object.
   * @param socket Reference to the native socket.
   */
  locked_socket_ptr(std::unique_lock<std::mutex> lock,
                    native_socket_type &socket)
      : lock_{std::move(lock)}, socket_{socket} {}

  /**
   * @brief Dereference operator to access the native socket.
   * @return Reference to the native socket.
   */
  [[nodiscard]] auto operator*() const noexcept -> native_socket_type & {
    return socket_;
  }

  /**
   * @brief Gets a pointer to the native socket.
   * @return A pointer to the native socket.
   */
  [[nodiscard]] auto get() const noexcept -> native_socket_type * {
    return &socket_;
  }

  /**
   * @brief Destructor.
   *
   * Releases the lock on the mutex when the object goes out of scope.
   */
  ~locked_socket_ptr() = default;

private:
  std::unique_lock<std::mutex> lock_;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  native_socket_type &socket_;
};

} // namespace iosched::socket
#endif // IOSCHED_LOCKED_SOCKET_PTR_HPP
