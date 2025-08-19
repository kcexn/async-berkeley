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
 * @file socket_message.hpp
 * @brief This file defines the `socket_message` class, a thread-safe container
 * for advanced socket I/O operations.
 */
#pragma once
#ifndef IO_SOCKET_MESSAGE_HPP
#define IO_SOCKET_MESSAGE_HPP
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include "platforms/windows/socket.hpp"
#else
#include "platforms/posix/socket.hpp"
#endif

#include "socket_address.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace io::socket {

/**
 * @brief Type alias for ancillary data buffer used in socket messages.
 *
 * This buffer stores control information that can be passed alongside the
 * main message data in advanced socket operations. Ancillary data can include
 * file descriptors, credentials, socket options, or other metadata that needs
 * to be transmitted with the message.
 *
 * @see sendmsg(2), recvmsg(2), cmsg(3)
 */
using ancillary_buffer = std::vector<char>;

/**
 * @brief Type alias for scatter-gather I/O buffer collection.
 *
 * This collection holds multiple data buffers that can be used for vectored
 * I/O operations. Scatter-gather I/O allows reading from or writing to
 * multiple non-contiguous memory regions in a single system call, improving
 * performance by reducing the number of system calls needed for complex
 * data structures.
 *
 * @see readv(2), writev(2), sendmsg(2), recvmsg(2)
 */
using scatter_gather_buffer = std::vector<socket_buffer_type>;

/**
 * @brief A data structure that contains all the components of a socket message.
 *
 * This structure holds the complete data for a socket message, including the
 * address, data buffers, control data, and flags. It is used internally by the
 * `socket_message` class for thread-safe storage.
 */
struct message_data {
  socket_address address; ///< The socket address for the message.
  scatter_gather_buffer
      buffers; ///< A collection of data buffers for scatter-gather I/O.
  ancillary_buffer control; ///< The ancillary data (control information).
  int flags{};              ///< The message flags for socket operations.
};

/**
 * @brief A thread-safe container for socket messages used in advanced I/O
 * operations.
 *
 * The `socket_message` class provides a thread-safe wrapper around the socket
 * message data, supporting scatter-gather I/O operations with ancillary data
 * and control information. This class is designed for use with the `sendmsg()`
 * and `recvmsg()` system calls, which require complex message structures.
 *
 * @details
 * Key features:
 * - Thread-safe access to all message components using mutex protection.
 * - Move-only semantics to prevent resource duplication.
 * - Eager initialization of internal data storage.
 * - Support for scatter-gather I/O with multiple data buffers.
 * - Handling of ancillary data for control messages.
 * - Management of socket addresses for message routing.
 *
 * @par Thread Safety
 * All operations on a `socket_message` instance are thread-safe due to internal
 * mutex protection. Multiple threads can safely access different
 * `socket_message` instances concurrently. Concurrent access to the same
 * instance is serialized.
 *
 * @note This class uses eager initialization, meaning the internal data
 * structure is created immediately in the constructor.
 *
 * @warning Copy operations are explicitly deleted to prevent ambiguity in
 * resource ownership. Use move semantics instead.
 */
class socket_message {

public:
  /**
   * @brief Default constructor.
   *
   * Creates an empty socket message with initialized internal data.
   */
  socket_message();

  /**
   * @brief Deleted copy constructor.
   *
   * Copying is disallowed to enforce move-only semantics and prevent resource
   * ownership issues.
   */
  socket_message(const socket_message &other) = delete;

  /**
   * @brief Deleted copy assignment operator.
   *
   * Copying is disallowed to enforce move-only semantics and prevent resource
   * ownership issues.
   */
  auto operator=(const socket_message &other) -> socket_message & = delete;

  /**
   * @brief Move constructor.
   *
   * Transfers ownership of the socket message data from another instance,
   * leaving the source instance in a valid but unspecified state.
   *
   * @param other The `socket_message` to move from.
   */
  socket_message(socket_message &&other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * Transfers ownership of the socket message data from another instance,
   * leaving the source instance in a valid but unspecified state.
   *
   * @param other The `socket_message` to move from.
   * @return A reference to this instance.
   */
  auto operator=(socket_message &&other) noexcept -> socket_message &;

  /**
   * @brief Swaps the contents of two `socket_message` instances.
   *
   * This function atomically swaps the contents of two `socket_message`
   * instances using a dual-mutex lock to prevent deadlocks.
   *
   * @param lhs The first `socket_message` instance.
   * @param rhs The second `socket_message` instance.
   *
   * @note This function uses `std::scoped_lock` to prevent deadlocks when
   * locking multiple mutexes.
   */
  friend auto swap(socket_message &lhs, socket_message &rhs) noexcept -> void;

  /**
   * @brief Gets the socket address.
   *
   * @return A copy of the socket address associated with this message.
   */
  auto address() const -> socket_address;

  /**
   * @brief Sets the socket address.
   *
   * @param address The socket address to set.
   * @return A reference to this `socket_message` for method chaining.
   */
  auto operator=(socket_address address) -> socket_message &;

  /**
   * @brief Gets the data buffers.
   *
   * These buffers are used for vectored I/O operations that can read or write
   * multiple non-contiguous memory regions in a single system call.
   *
   * @return A copy of the scatter-gather buffer collection.
   */
  [[nodiscard]] auto buffers() const -> scatter_gather_buffer;

  /**
   * @brief Sets the data buffers.
   *
   * The provided buffers are moved into the message, transferring ownership.
   *
   * @param buffers The buffer collection to set (will be moved).
   * @return A reference to this `socket_message` for method chaining.
   */
  auto set_buffers(scatter_gather_buffer buffers) -> socket_message &;

  /**
   * @brief Exchanges the data buffers.
   *
   * The provided buffers are moved into the message, transferring ownership.
   * The previous buffers are returned to the caller.
   *
   * @param buffers The buffer collection to set (will be moved).
   * @return The previous buffer collection that was replaced.
   */
  auto exchange_buffers(scatter_gather_buffer buffers) -> scatter_gather_buffer;

  /**
   * @brief Gets the ancillary data.
   *
   * This data contains control information that can be passed alongside the
   * main message data, such as file descriptors, credentials, or other
   * metadata.
   *
   * @return A copy of the ancillary data.
   */
  [[nodiscard]] auto control() const -> ancillary_buffer;

  /**
   * @brief Sets the ancillary data.
   *
   * The provided data is moved into the message, transferring ownership.
   *
   * @param control The ancillary data to set (will be moved).
   * @return A reference to this `socket_message` for method chaining.
   */
  auto set_control(ancillary_buffer control) -> socket_message &;

  /**
   * @brief Exchanges the ancillary data.
   *
   * The provided control data is moved into the message, transferring
   * ownership. The previous control data is returned to the caller.
   *
   * @param control The ancillary data to set (will be moved).
   * @return The previous ancillary data that was replaced.
   */
  auto exchange_control(ancillary_buffer control) -> ancillary_buffer;

  /**
   * @brief Gets the message flags.
   *
   * These flags control the behavior of socket operations and correspond to the
   * flags parameter used with the `sendmsg()` and `recvmsg()` system calls.
   *
   * @return The message flags.
   */
  [[nodiscard]] auto flags() const -> int;

  /**
   * @brief Sets the message flags.
   *
   * These flags control the behavior of socket operations and correspond to the
   * flags parameter used with the `sendmsg()` and `recvmsg()` system calls
   * (e.g., `MSG_DONTWAIT`, `MSG_PEEK`, `MSG_TRUNC`).
   *
   * @param flags The message flags to set.
   * @return A reference to this `socket_message` for method chaining.
   */
  auto set_flags(int flags) -> socket_message &;

  /**
   * @brief Exchanges the message flags.
   *
   * The provided flags are set in the message, replacing the current flags.
   * The previous flags are returned to the caller.
   *
   * @param flags The message flags to set.
   * @return The previous message flags that were replaced.
   */
  auto exchange_flags(int flags) -> int;

  /**
   * @brief Default destructor.
   *
   * Cleans up internal resources. The `unique_ptr` ensures that the
   * `message_data` structure is properly deallocated.
   */
  ~socket_message() = default;

private:
  /// The storage for the message data.
  std::unique_ptr<message_data> data_;
  /// A mutex for thread-safe access to the message data.
  mutable std::mutex mtx_;
};

// TODO implement customization points for sendmsg and recvmsg that are passed
// in socket_message types.

} // namespace io::socket

#endif // IO_SOCKET_MESSAGE_HPP
