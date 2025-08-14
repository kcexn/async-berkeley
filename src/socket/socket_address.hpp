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
 * @file socket_address.hpp
 * @brief Provides a platform-independent socket address abstraction.
 */
#pragma once
#ifndef IOSCHED_SOCKET_ADDRESS_HPP
#define IOSCHED_SOCKET_ADDRESS_HPP
#include "socket.hpp"

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace iosched::socket {
/**
 * @class socket_address
 * @brief Represents a generic socket address.
 *
 * This class provides a platform-independent way to store socket address
 * information, including the address data and its size. It uses
 * `sockaddr_storage_type` for the address data and `socklen_type` for the size,
 * which are platform-specific aliases.
 */
class socket_address {

public:
  /**
   * @brief Constructs a socket_address from a native socket address structure.
   *
   * This constructor copies the provided native socket address data into its
   * internal storage, making it suitable for passing to socket functions.
   *
   * @param addr A pointer to the native socket address (e.g., `sockaddr_in`).
   * @param size The size of the address structure in bytes.
   */
  explicit socket_address(const sockaddr_type *addr,
                          socklen_type size) noexcept;

  /**
   * @brief Gets a mutable pointer to the underlying socket address data.
   *
   * This allows the raw address data to be modified, which is necessary for
   * functions like `::accept()` or `::recvfrom()` that populate a socket
   * address structure provided by the caller.
   *
   * @return A pointer to the `sockaddr_type` data.
   */
  [[nodiscard]] auto data() noexcept -> sockaddr_type *;

  /**
   * @brief Gets a constant pointer to the underlying socket address data.
   *
   * This provides read-only access to the address data, suitable for passing
   * to functions like `::connect()` or `::bind()` that do not modify the
   * address.
   *
   * @return A constant pointer to the `sockaddr_type` data.
   */
  [[nodiscard]] auto data() const noexcept -> const sockaddr_type *;

  /**
   * @brief Gets a mutable pointer to the size of the socket address.
   *
   * This allows the size to be modified, which is necessary for functions
   * like `::accept()` or `::recvfrom()` that update the size argument to
   * reflect the actual size of the returned address.
   *
   * @return A pointer to the `socklen_type` size.
   */
  [[nodiscard]] auto size() noexcept -> socklen_type *;

  /**
   * @brief Gets a constant pointer to the size of the socket address.
   *
   * This provides read-only access to the size, suitable for functions that
   * only need to know the size of the address being passed.
   *
   * @return A constant pointer to the `socklen_type` size.
   */
  [[nodiscard]] auto size() const noexcept -> const socklen_type *;

  /**
   * @brief Compares two socket_address objects for equality.
   *
   * Two addresses are considered equal if they have the same size and their
   * underlying address data is identical. This is determined by a memory
   * comparison of the storage.
   *
   * @param other The socket_address to compare against.
   * @return `true` if the addresses are equal, `false` otherwise.
   */
  auto operator==(const socket_address &other) const noexcept -> bool;

private:
  sockaddr_storage_type storage_{};
  socklen_type size_{};
};

} // namespace iosched::socket

#endif // IOSCHED_SOCKET_ADDRESS_HPP
