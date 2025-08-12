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
 * @file socket_buffer.hpp
 * @brief Defines a thread-safe, buffered socket interface.
 *
 * This file provides a set of classes for managing socket I/O buffers using
 * a diamond inheritance pattern to separate read and write concerns.
 */
#pragma once
#ifndef IOSCHED_SOCKET_BUFFER_HPP
#define IOSCHED_SOCKET_BUFFER_HPP

#include "../iosched.hpp"
#include "../socket/socket_message.hpp"

#include <memory>
#include <mutex>
#include <queue>

/**
 * @namespace iosched::buffers
 * @brief Contains classes for buffer management, particularly for sockets.
 */
namespace iosched::buffers {

/**
 * @class socket_buffer_base
 * @brief Base class for socket buffers, managing the socket and I/O queues.
 *
 * This class holds the common state for a socket buffer, including the native
 * socket handle, separate read and write message queues, and a single mutex
 * to ensure thread-safe access to all shared resources. It is designed to be
 * used as a virtual base class to prevent the diamond problem in derived
 * classes that combine read and write functionality.
 */
class socket_buffer_base {
public:
  /// @brief Platform-specific native socket handle type.
  using native_socket_type = ::iosched::native_socket_type;
  /// @brief The message type used for I/O operations.
  using socket_message = ::iosched::socket::socket_message;
  /// @brief The underlying container for the read and write buffers.
  using buffer_type = std::queue<std::shared_ptr<socket_message>>;

  /**
   * @brief Constructs a socket buffer.
   * @param sock The native socket handle to manage.
   * @param rbuf The initial read buffer.
   * @param wbuf The initial write buffer.
   */
  socket_buffer_base(native_socket_type sock = ::iosched::INVALID_SOCKET,
                     buffer_type rbuf = {}, buffer_type wbuf = {});
  /**
   * @brief Copy constructs a socket buffer.
   * @param other The object to copy from.
   * @note The other object's mutex is locked during the copy operation to
   * ensure thread-safe access to its data.
   */
  socket_buffer_base(const socket_buffer_base &other);
  /**
   * @brief Move constructs a socket buffer.
   * @param other The object to move from.
   * @note The other object's mutex is locked during the move operation to
   * ensure thread-safe access to its data.
   */
  socket_buffer_base(socket_buffer_base &&other) noexcept;
  /**
   * @brief Copy assigns a socket buffer.
   * @param other The object to copy from.
   * @return A reference to this object.
   * @note This operation is thread-safe, locking both this and the other
   *       object's mutexes to prevent deadlocks.
   */
  auto operator=(const socket_buffer_base &other) -> socket_buffer_base &;
  /**
   * @brief Move assigns a socket buffer.
   * @param other The object to move from.
   * @return A reference to this object.
   * @note This operation is thread-safe, locking both this and the other
   *       object's mutexes to prevent deadlocks.
   */
  auto operator=(socket_buffer_base &&other) noexcept -> socket_buffer_base &;

  /**
   * @brief Swaps the contents of two socket_buffer_base objects.
   * @param lhs The first object to swap.
   * @param rhs The second object to swap.
   * @note This function is `noexcept` and thread-safe, locking both objects'
   *       mutexes to perform the swap atomically.
   */
  friend auto swap(socket_buffer_base &lhs,
                   socket_buffer_base &rhs) noexcept -> void;

  /**
   * @brief Virtual destructor.
   *
   * Ensures that derived classes are properly destroyed.
   */
  virtual ~socket_buffer_base() = default;

protected:
  // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
  /// @brief The native socket handle.
  native_socket_type socket;
  /// @brief The buffer for incoming messages.
  buffer_type read_buffer;
  /// @brief The buffer for outgoing messages.
  buffer_type write_buffer;
  /// @brief Mutex for thread-safe access to the socket and buffers.
  mutable std::mutex mtx;
  // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
};

/**
 * @struct socket_read_buffer
 * @brief Provides the read interface for a socket buffer.
 *
 * Inherits virtually from `socket_buffer_base` to provide read-only
 * operations on the shared socket and buffers.
 */
struct socket_read_buffer : public virtual socket_buffer_base {
  /**
   * @brief Reads a message from the socket's read buffer.
   * @return A shared pointer to the message, or nullptr if the buffer is empty.
   */
  auto read() -> std::shared_ptr<socket_message>;
};

/**
 * @struct socket_write_buffer
 * @brief Provides the write interface for a socket buffer.
 *
 * Inherits virtually from `socket_buffer_base` to provide write-only
 * operations on the shared socket and buffers.
 */
struct socket_write_buffer : public virtual socket_buffer_base {
  /**
   * @brief Writes a message to the socket's write buffer.
   * @param msg The message to write.
   */
  auto write(socket_message msg) -> void;
};

/**
 * @struct socket_buffer
 * @brief A full-duplex socket buffer with both read and write capabilities.
 *
 * This class combines the read and write interfaces into a single object
 * by inheriting from both `socket_read_buffer` and `socket_write_buffer`.
 * The virtual inheritance from `socket_buffer_base` ensures a single instance
 * of the base class state.
 */
struct socket_buffer : public socket_read_buffer, public socket_write_buffer {};

} // namespace iosched::buffers
#endif // IOSCHED_SOCKET_BUFFER_HPP
