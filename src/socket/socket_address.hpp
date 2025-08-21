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
 * @brief This file defines the `socket_address` class, a platform-independent
 * abstraction for socket addresses.
 */
#pragma once
#ifndef IO_SOCKET_ADDRESS_HPP
#define IO_SOCKET_ADDRESS_HPP
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include "platforms/windows/socket.hpp"
#else
#include "platforms/posix/socket.hpp"
#endif

namespace io::socket {
/**
 * @brief A platform-independent representation of a socket address.
 *
 * This class provides a wrapper around the native socket address structures,
 * offering a safe and convenient way to manage address information for socket
 * operations. It uses `sockaddr_storage_type` to ensure sufficient space for
 * any address family and `socklen_type` for the address size, which are
 * platform-specific aliases.
 */
class socket_address {

public:
  /**
   * @brief Constructs an empty socket address.
   *
   * The internal storage is zero-initialized, and the size is set to the full
   * capacity of the address storage. This prepares the object to be populated
   * by functions that require a buffer for the address, such as `accept()` or
   * `recvfrom()`.
   */
  socket_address() = default;

  /** @brief Default copy constructor. */
  socket_address(const socket_address &other) = default;

  /** @brief Default move constructor. */
  socket_address(socket_address &&other) noexcept = default;

  /** @brief Default copy assignment operator. */
  auto operator=(const socket_address &other) -> socket_address & = default;

  /** @brief Default move assignment operator. */
  auto operator=(socket_address &&other) noexcept -> socket_address & = default;

  /**
   * @brief Constructs a socket address with a specific size.
   *
   * This constructor initializes the socket address to be ready to hold a
   * socket address of the specified size. The internal storage is
   * zero-initialized, and the size is set to the provided value. This is useful
   * when you know the exact size needed for a particular address family.
   *
   * @param size The size in bytes for the socket address structure.
   */
  constexpr explicit socket_address(socklen_type size) noexcept;

  /**
   * @brief Constructs a `socket_address` from a native socket address
   * structure.
   *
   * This constructor copies the provided native socket address data into its
   * internal storage, making it suitable for passing to socket functions that
   * require an address, such as `connect()` or `bind()`.
   *
   * @param addr A pointer to the native socket address (e.g., `sockaddr_in`).
   * The data pointed to is copied into the object.
   * @param size The size of the address structure in bytes.
   */
  socket_address(const sockaddr_type *addr, socklen_type size) noexcept;

  /**
   * @brief Returns a mutable pointer to the underlying socket address data.
   *
   * This allows the raw address data to be modified by functions like
   * `accept()` or `getpeername()` that populate a socket address structure
   * provided by the caller. The lifetime of the returned pointer is tied to
   * this `socket_address` object.
   *
   * @return A non-owning pointer to the `sockaddr_type` data.
   */
  [[nodiscard]] auto data() noexcept -> sockaddr_type *;

  /**
   * @brief Returns a constant pointer to the underlying socket address data.
   *
   * This provides read-only access to the address data, suitable for passing to
   * functions like `connect()` or `bind()` that do not modify the address. The
   * lifetime of the returned pointer is tied to this `socket_address` object.
   *
   * @return A non-owning constant pointer to the `sockaddr_type` data.
   */
  [[nodiscard]] auto data() const noexcept -> const sockaddr_type *;

  /**
   * @brief Returns a mutable pointer to the size of the socket address.
   *
   * This allows the size to be modified by functions like `accept()` or
   * `getpeername()` that update the size argument to reflect the actual size of
   * the returned address. The lifetime of the returned pointer is tied to this
   * `socket_address` object.
   *
   * @return A non-owning pointer to the `socklen_type` size.
   */
  [[nodiscard]] constexpr auto size() noexcept -> socklen_type *;

  /**
   * @brief Returns a constant pointer to the size of the socket address.
   *
   * This provides read-only access to the size, suitable for functions that
   * only need to know the size of the address being passed. The lifetime of the
   * returned pointer is tied to this `socket_address` object.
   *
   * @return A non-owning constant pointer to the `socklen_type` size.
   */
  [[nodiscard]] constexpr auto size() const noexcept -> const socklen_type *;

  /**
   * @brief Compares two `socket_address` objects for equality.
   *
   * Two addresses are considered equal if they have the same size and their
   * underlying address data is bit-for-bit identical.
   *
   * @param other The `socket_address` to compare against.
   * @return `true` if the addresses are equal, `false` otherwise.
   */
  constexpr auto operator==(const socket_address &other) const noexcept -> bool;

  /** @brief Default destructor. */
  ~socket_address() = default;

private:
  sockaddr_storage_type storage_{};
  socklen_type size_{sizeof(storage_)};
};

/**
 * @brief Creates a `socket_address` object, optionally initialized from a
 * native address.
 * @relates socket_address
 *
 * This function is a helper to construct a `socket_address` instance. If `addr`
 * is provided, the `socket_address` is initialized with the given native
 * address and its size. Otherwise, it creates a `socket_address` with an
 * appropriate size for `SockAddr` but without initial address data. It performs
 * a compile-time assertion to guarantee that `SockAddr` can fit within
 * `sockaddr_storage_type`.
 *
 * @tparam SockAddr The native socket address structure type (e.g.,
 * `sockaddr_in`, `sockaddr_in6`).
 * @param addr An optional pointer to a native socket address structure to
 * initialize from. If `nullptr`, an empty `socket_address` of the appropriate
 * size is created.
 * @return A `socket_address` object, either initialized from `addr` or with the
 * size of `SockAddr`.
 */
template <typename SockAddr>
constexpr auto make_address(const SockAddr *addr = nullptr) noexcept -> socket_address {
  static_assert(sizeof(SockAddr) <= sizeof(sockaddr_storage_type),
                "SockAddr must fit into sockaddr_storage_type.");

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return addr ? socket_address{reinterpret_cast<const sockaddr_type *>(addr),
                               sizeof(SockAddr)}
              : socket_address{sizeof(SockAddr)};
}

} // namespace io::socket

#endif // IO_SOCKET_ADDRESS_HPP
