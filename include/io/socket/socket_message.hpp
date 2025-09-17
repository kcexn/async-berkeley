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
#include "io/config.h"
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
  std::span<std::byte> msg_name;
  /**
   * @brief I/O vectors for scatter/gather operations.
   */
  std::span<native_buffer_type> msg_iov;
  /**
   * @brief Ancillary data (control information).
   */
  std::span<std::byte> msg_control;
  /**
   * @brief Flags on the received message.
   */
  int flags{};

  /**
   * @brief Converts the message header to the native socket message type.
   */
  [[nodiscard]] explicit operator socket_message_type() noexcept;
};

/**
 * @brief A container for managing buffers for scatter-gather I/O operations.
 *
 * This class provides a convenient way to handle a collection of buffers
 * that can be used in readv/writev (scatter/gather) style I/O operations.
 */
class scatter_gather_buffers {
public:
  /**
   * @brief Adds a buffer to the collection.
   * @tparam B The type of the buffer, which must satisfy the ScatterGatherLike
   * concept.
   * @param buf The buffer to add.
   */
  template <ScatterGatherLike B> constexpr auto push_back(const B &buf) -> void
  {
    using element_type =
        std::remove_pointer_t<decltype(std::ranges::data(buf))>;
    using pointer_type = std::decay_t<element_type> *;
#if OS_WINDOWS
    buffers_.push_back({std::ranges::size(buf) * sizeof(element_type),
                        reinterpret_cast<char *>(
                            const_cast<pointer_type>(std::ranges::data(buf)))});
#else
    buffers_.push_back({const_cast<pointer_type>(std::ranges::data(buf)),
                        std::ranges::size(buf) * sizeof(element_type)});
#endif // OS_WINDOWS
  }
  /**
   * @brief Adds a native buffer to the collection.
   * @param buf The native buffer to add.
   */
  constexpr auto push_back(native_buffer_type buf) -> void
  {
    buffers_.push_back(buf);
  }
  /**
   * @brief Constructs a native buffer in-place at the end of the collection.
   * @tparam Args The types of the arguments to forward to the constructor of
   * the native buffer.
   * @param args The arguments to forward.
   * @return A reference to the constructed buffer.
   */
  template <typename... Args>
  constexpr auto emplace_back(Args &&...args) -> decltype(auto)
  {
    return buffers_.emplace_back(std::forward<Args>(args)...);
  }
  /**
   * @brief Returns an iterator to the beginning of the buffer collection.
   */
  [[nodiscard]] constexpr auto begin() noexcept -> decltype(auto)
  {
    return buffers_.begin();
  }
  /**
   * @brief Returns a const iterator to the beginning of the buffer collection.
   */
  [[nodiscard]] constexpr auto begin() const noexcept -> decltype(auto)
  {
    return buffers_.cbegin();
  }
  /**
   * @brief Returns an iterator to the end of the buffer collection.
   */
  [[nodiscard]] constexpr auto end() noexcept -> decltype(auto)
  {
    return buffers_.end();
  }
  /**
   * @brief Returns a const iterator to the end of the buffer collection.
   */
  [[nodiscard]] constexpr auto end() const noexcept -> decltype(auto)
  {
    return buffers_.cend();
  }
  /**
   * @brief Returns the number of buffers in the collection.
   */
  [[nodiscard]] constexpr auto size() const noexcept -> decltype(auto)
  {
    return buffers_.size();
  }
  /**
   * @brief Checks if the buffer collection is empty.
   */
  [[nodiscard]] constexpr auto empty() const noexcept -> decltype(auto)
  {
    return buffers_.empty();
  }
  /**
   * @brief Checks if the buffer collection is not empty.
   * @return True if not empty, false otherwise.
   */
  [[nodiscard]] constexpr operator bool() const noexcept
  {
    return !buffers_.empty();
  }
  /**
   * @brief Advances the buffers by a specified length.
   *
   * This is useful for handling partial reads/writes. It adjusts the internal
   * buffers to skip the given number of bytes.
   *
   * @param len The number of bytes to advance.
   * @return A reference to this scatter_gather_buffers object.
   */
  auto operator+=(std::size_t len) noexcept -> scatter_gather_buffers &;

private:
  std::vector<native_buffer_type> buffers_;
};

/**
 * @brief Represents a complete socket message.
 *
 * This structure extends `message_header` with storage for buffers and
 * control data.
 */
template <SocketAddress Addr = sockaddr_storage_type> struct socket_message {
  /** The socket address type. */
  using address_type = socket_address<Addr>;
  /**
   * @brief Optional address of the sender/receiver.
   */
  std::optional<address_type> address;
  /**
   * @brief Buffers for scatter/gather I/O.
   */
  scatter_gather_buffers buffers;
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
  [[nodiscard]] explicit operator socket_message_type() noexcept
  {
    message_header header = {
        .msg_iov = buffers, .msg_control = control, .flags = flags};
    if (address)
      header.msg_name = *address;
    return static_cast<socket_message_type>(header);
  }
};

} // namespace io::socket

#endif // IO_SOCKET_MESSAGE_HPP
