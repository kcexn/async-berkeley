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

#pragma once
#ifndef IO_SOCKET_MESSAGE_HPP
#define IO_SOCKET_MESSAGE_HPP
#include "socket.hpp"
#include "socket_address.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace io::socket {

/**
 * @brief Data structure containing all components of a socket message
 *
 * This structure holds the complete data for a socket message including
 * address information, data buffers, control data, and message flags.
 * Used internally by socket_message for thread-safe storage.
 */
struct message_data {
  /// Type alias for scatter-gather I/O buffer collection
  using scatter_gather_type = std::vector<socket_buffer_type>;
  /// Type alias for ancillary data storage
  using ancillary_data_type = std::vector<char>;

  socket_address address;        ///< Socket address for the message
  scatter_gather_type buffers;   ///< Collection of data buffers for scatter-gather I/O
  ancillary_data_type control;   ///< Ancillary data (control information)
  int flags{};                   ///< Message flags for socket operations
};

/**
 * @brief Thread-safe socket message container for advanced I/O operations
 *
 * The socket_message class provides a thread-safe wrapper around socket message
 * data, supporting scatter-gather I/O operations with ancillary data and control
 * information. This class is designed for use with sendmsg() and recvmsg()
 * system calls that require complex message structures.
 *
 * @details
 * Key features:
 * - Thread-safe access to all message components using mutex protection
 * - Move-only semantics preventing resource duplication
 * - Eager initialization of internal data storage
 * - Support for scatter-gather I/O with multiple data buffers
 * - Ancillary data handling for control messages
 * - Socket address management for message routing
 *
 * Thread Safety:
 * - All operations are thread-safe using internal mutex protection
 * - Multiple threads can safely access different socket_message instances
 * - Concurrent access to the same instance is serialized
 *
 * @note This class uses eager initialization - the internal data structure
 *       is created immediately in the constructor.
 *
 * @warning Copy operations are explicitly deleted to prevent resource
 *          ownership ambiguity. Use move semantics instead.
 */
class socket_message {

public:
  /**
   * @brief Default constructor
   *
   * Creates an empty socket message with initialized internal data.
   * Data is immediately initialized in the constructor.
   */
  socket_message();

  /**
   * @brief Deleted copy constructor
   *
   * Copy construction is explicitly deleted to enforce move-only semantics
   * and prevent resource ownership ambiguity.
   */
  socket_message(const socket_message &other) = delete;

  /**
   * @brief Deleted copy assignment operator
   *
   * Copy assignment is explicitly deleted to enforce move-only semantics
   * and prevent resource ownership ambiguity.
   */
  auto operator=(const socket_message &other) -> socket_message & = delete;

  /**
   * @brief Move constructor
   *
   * Transfers ownership of the socket message data from another instance.
   * The source instance is left in a valid but unspecified state.
   *
   * @param other The socket_message to move from
   */
  socket_message(socket_message &&other) noexcept;

  /**
   * @brief Move assignment operator
   *
   * Transfers ownership of the socket message data from another instance.
   * The source instance is left in a valid but unspecified state.
   *
   * @param other The socket_message to move from
   * @return Reference to this instance
   */
  auto operator=(socket_message &&other) noexcept -> socket_message &;

  /**
   * @brief Thread-safe swap operation
   *
   * Atomically swaps the contents of two socket_message instances using
   * dual-mutex locking to prevent deadlocks. Both instances are locked
   * simultaneously to ensure atomicity.
   *
   * @param lhs First socket_message instance
   * @param rhs Second socket_message instance
   *
   * @note Uses std::scoped_lock to prevent deadlocks when locking multiple mutexes
   */
  friend auto swap(socket_message &lhs, socket_message &rhs) noexcept -> void;

  /**
   * @brief Get the socket address
   *
   * Returns a copy of the socket address associated with this message.
   *
   * @return Copy of the socket address
   */
  auto address() const -> socket_address;

  /**
   * @brief Set the socket address
   *
   * Sets the socket address for this message.
   *
   * @param address The socket address to set
   * @return Reference to this socket_message for method chaining
   */
  auto operator=(socket_address address) -> socket_message &;

  /**
   * @brief Get the data buffers
   *
   * Returns a copy of the scatter-gather buffer collection. These buffers
   * are used for vectored I/O operations that can read/write multiple
   * non-contiguous memory regions in a single system call.
   *
   * @return Copy of the scatter-gather buffer collection
   */
  auto buffers() const -> message_data::scatter_gather_type;

  /**
   * @brief Set the data buffers
   *
   * Sets the scatter-gather buffer collection for this message. The buffers
   * are moved into the message, transferring ownership.
   *
   * @param buffers The buffer collection to set (will be moved)
   * @return Reference to this socket_message for method chaining
   */
  auto operator=(message_data::scatter_gather_type buffers) -> socket_message &;

  /**
   * @brief Get the ancillary data
   *
   * Returns a copy of the ancillary (control) data. This data contains
   * control information that can be passed alongside the main message data,
   * such as file descriptors, credentials, or other metadata.
   *
   * @return Copy of the ancillary data
   */
  auto control() const -> message_data::ancillary_data_type;

  /**
   * @brief Set the ancillary data
   *
   * Sets the ancillary (control) data for this message. The data is moved
   * into the message, transferring ownership.
   *
   * @param control The ancillary data to set (will be moved)
   * @return Reference to this socket_message for method chaining
   */
  auto operator=(message_data::ancillary_data_type control) -> socket_message &;

  /**
   * @brief Get the message flags
   *
   * Returns the message flags that control the behavior of socket operations.
   * These flags correspond to the flags parameter used with sendmsg() and
   * recvmsg() system calls.
   *
   * @return The message flags
   */
  auto flags() const -> int;

  /**
   * @brief Set the message flags
   *
   * Sets the message flags that control the behavior of socket operations.
   * These flags correspond to the flags parameter used with sendmsg() and
   * recvmsg() system calls (e.g., MSG_DONTWAIT, MSG_PEEK, MSG_TRUNC).
   *
   * @param flags The message flags to set
   * @return Reference to this socket_message for method chaining
   */
  auto operator=(int flags) -> socket_message &;

  /**
   * @brief Default destructor
   *
   * Automatically cleans up internal resources. The unique_ptr ensures
   * proper cleanup of the message_data structure.
   */
  ~socket_message() = default;

private:
  /// Message data storage
  std::unique_ptr<message_data> data_;
  /// Mutex for thread-safe access to message data
  mutable std::mutex mtx_;
};

// TODO implement customization points for sendmsg and recvmsg that are passed in socket_message types.

} // namespace io::socket

#endif // IO_SOCKET_MESSAGE_HPP
