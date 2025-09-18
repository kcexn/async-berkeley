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

#include <memory>
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
  /** @brief Optional address of the sender/receiver. */
  std::span<std::byte> msg_name;
  /** @brief I/O vectors for scatter/gather operations. */
  std::span<native_buffer_type> msg_iov;
  /** @brief Ancillary data (control information). */
  std::span<std::byte> msg_control;
  /** @brief Flags on the received message. */
  int flags{};

  /** @brief Converts the message header to the native socket message type. */
  [[nodiscard]] explicit operator socket_message_type() noexcept;
};

/**
 * @brief A container for managing buffers for scatter-gather I/O operations.
 *
 * This class provides a convenient way to handle a collection of buffers
 * that can be used in readv/writev (scatter/gather) style I/O operations.
 */
template <AllocatorLike Allocator = std::allocator<native_buffer_type>>
class message_buffer {
public:
  /** @brief The allocator type. */
  using allocator_type = std::allocator_traits<
      Allocator>::template rebind_alloc<native_buffer_type>;
  /** @brief The underlying buffer type. */
  using buffer_type = std::vector<native_buffer_type, allocator_type>;
  /** @brief Iterator for the buffer. */
  using iterator = typename buffer_type::iterator;
  /** @brief Constant iterator for the buffer. */
  using const_iterator = typename buffer_type::const_iterator;
  /** @brief Size type for the buffer. */
  using size_type = typename buffer_type::size_type;

  /**
   * @brief Default construct a new message buffer object
   * @param alloc The allocator to use for the buffer.
   */
  constexpr message_buffer(
      const allocator_type &alloc =
          allocator_type()) noexcept(noexcept(allocator_type()));

  /**
   * @brief Adds a buffer to the collection.
   * @tparam B The type of the buffer, which must satisfy the ScatterGatherLike
   * concept.
   * @param buf The buffer to add.
   */
  template <ScatterGatherLike B> constexpr auto push_back(const B &buf) -> void;

  /**
   * @brief Adds a native buffer to the collection.
   * @param buf The native buffer to add.
   */
  constexpr auto push_back(native_buffer_type buf) -> void;

  /**
   * @brief Constructs a native buffer in-place at the end of the collection.
   * @tparam Args The types of the arguments to forward to the constructor of
   * the native buffer.
   * @param args The arguments to forward.
   * @return A reference to the constructed buffer.
   */
  template <typename... Args>
  constexpr auto emplace_back(Args &&...args) -> decltype(auto);

  /** @brief Returns an iterator to the beginning of the buffer collection. */
  [[nodiscard]] constexpr auto begin() noexcept -> iterator;

  /** @brief Returns a const iterator to the beginning of the buffer collection.
   */
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator;

  /** @brief Returns an iterator to the end of the buffer collection. */
  [[nodiscard]] constexpr auto end() noexcept -> iterator;

  /** @brief Returns a const iterator to the end of the buffer collection. */
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator;

  /** @brief Returns the number of buffers in the collection. */
  [[nodiscard]] constexpr auto size() const noexcept -> size_type;

  /** @brief Checks if the buffer collection is empty. */
  [[nodiscard]] constexpr auto empty() const noexcept -> bool;

  /**
   * @brief Checks if the buffer collection is not empty.
   * @return True if not empty, false otherwise.
   */
  [[nodiscard]] explicit constexpr operator bool() const noexcept;

  /**
   * @brief Advances the buffers by a specified length.
   *
   * This is useful for handling partial reads/writes. It adjusts the internal
   * buffers to skip the given number of bytes.
   *
   * @param len The number of bytes to advance.
   * @return A reference to this scatter_gather_buffers object.
   */
  auto operator+=(std::size_t len) noexcept -> message_buffer &;

private:
  buffer_type buffer_;
};

/**
 * @brief Represents a complete socket message.
 *
 * This structure extends `message_header` with storage for buffers and
 * control data.
 */
template <SocketAddress Addr = sockaddr_storage_type,
          AllocatorLike Allocator = std::allocator<char>>
struct socket_message {
  /** @brief The allocator type for message buffers. */
  using message_allocator = std::allocator_traits<
      Allocator>::template rebind_alloc<native_buffer_type>;
  /** @brief The allocator type for control data. */
  using control_allocator =
      std::allocator_traits<Allocator>::template rebind_alloc<std::byte>;
  /** @brief The socket address type. */
  using address_type = std::optional<socket_address<Addr>>;
  /** @brief The message buffer type. */
  using message_type = message_buffer<message_allocator>;
  /** @brief The control buffer type. */
  using control_type = std::vector<std::byte, control_allocator>;

  /** @brief Optional address of the sender/receiver. */
  address_type address;

  /** @brief Buffers for scatter/gather I/O. */
  message_type buffers;

  /** @brief Ancillary data (control information). */
  control_type control;

  /** @brief Flags on the received message. */
  int flags{};

  /** @brief Converts the socket message to the native socket message type. */
  [[nodiscard]] explicit operator socket_message_type() noexcept;
};

} // namespace io::socket

#include "impl/socket_message_impl.hpp" // IWYU pragma: export

#endif // IO_SOCKET_MESSAGE_HPP
