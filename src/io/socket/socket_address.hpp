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
 * @brief Defines the `socket_address` class for platform-independent socket
 * address management.
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
#include "io/detail/concepts.hpp"
#include "socket_option.hpp"

namespace io::socket {
/**
 * @brief Represents a platform-independent socket address.
 *
 * This class inherits from `socket_option` to provide a generic way to handle
 * different socket address types.
 */
template <SocketAddress Addr = sockaddr_storage_type>
struct socket_address : public socket_option<Addr> {
  using Base = socket_option<Addr>;
  using Base::Base;

  /**
   * @brief Constructs a socket_address from a raw socket address structure.
   *
   * @tparam Size The size of the socket address structure.
   * @param addr A pointer to the raw socket address structure.
   * @param size The size of the socket address structure.
   */
  template <socklen_type Size = sizeof(Addr)>
    requires(Size <= sizeof(Addr))
  socket_address(const sockaddr_type *addr, socklen_type size = Size) noexcept
      : Base(std::span<const std::byte, Size>(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const std::byte *>(addr), size)) {}

  /**
   * @brief Constructs a socket_address from another socket_address.
   *
   * @tparam OtherAddr The type of the other socket address.
   * @param other The other socket_address to construct from.
   */
  template <SocketAddress OtherAddr>
  socket_address(const socket_address<OtherAddr> &other) noexcept
      : Base(std::span<const std::byte, sizeof(OtherAddr)>(other)) {}
};

/**
 * @brief Creates a `socket_address` from a socket address structure.
 * @tparam SockAddr The type of the socket address structure.
 * @param addr A pointer to the socket address structure.
 * @return A `socket_address` object.
 */
template <SocketAddress Addr = sockaddr_storage_type>
auto make_address(const Addr *addr = nullptr) -> socket_address<Addr> {
  if (!addr)
    return {};

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return {reinterpret_cast<const sockaddr_type *>(addr), sizeof(Addr)};
}

using socket_address_storage = socket_address<sockaddr_storage_type>;

} // namespace io::socket
#endif // IO_SOCKET_ADDRESS_HPP
