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
 * @brief Defines structures for handling socket messages.
 */
#pragma once
#ifndef IO_SOCKET_MESSAGE_HPP
#define IO_SOCKET_MESSAGE_HPP
#include "detail/socket.hpp"
#include "socket_address.hpp"

#include <optional>
#include <vector>
namespace io::socket {
/**
 * @brief Represents the header of a socket message.
 *
 * This structure holds information about a socket message, including the
 * address, I/O vectors, and control data.
 */
struct message_header {
  /**
   * @brief Optional address of the sender/receiver.
   */
  std::span<std::byte> name;
  /**
   * @brief I/O vectors for scatter/gather operations.
   */
  std::span<buffer_type> iov;
  /**
   * @brief Ancillary data (control information).
   */
  std::span<std::byte> control;
  /**
   * @brief Flags on the received message.
   */
  int flags{};

  /**
   * @brief Converts the message header to the native socket message type.
   */
  explicit operator socket_message_type() noexcept;
};

/**
 * @brief Represents a complete socket message.
 *
 * This structure extends `message_header` with storage for buffers and
 * control data.
 */
struct socket_message {
  /**
   * @brief Optional address of the sender/receiver.
   */
  std::optional<socket_address<sockaddr_storage_type>> address;
  /**
   * @brief Buffers for scatter/gather I/O.
   */
  std::vector<buffer_type> buffers;
  /**
   * @brief Ancillary data (control information).
   */
  std::vector<std::byte> control;
  /**
   * @brief Flags on the received message.
   */
  int flags{};

  /**
   * @brief Converts the socket message to the native socket message type.
   */
  explicit operator socket_message_type() noexcept;
};

} // namespace io::socket

#endif // IO_SOCKET_MESSAGE_HPP
