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
 * @file operations.hpp
 * @brief Defines the available socket operations using the tag_invoke
 * customization point.
 */
#pragma once
#ifndef IO_OPERATIONS_HPP
#define IO_OPERATIONS_HPP
#include <boost/predef.h>
#if BOOST_OS_WINDOWS
#include "io/socket/platforms/windows/socket.hpp"
#else
#include "io/socket/platforms/posix/socket.hpp"
#endif
#include "tags.hpp"

#include <cerrno>
#include <type_traits>

namespace io::socket {
/**
 * @brief Concept for types that behave like a socket.
 * @tparam Socket The type to check.
 */
template <typename Socket>
concept SocketLike = requires {
  Socket{native_socket_type{}};
  std::is_convertible_v<Socket, native_socket_type>;
};

/**
 * @brief Concept for types that behave like a socket message.
 * @tparam Message The type to check.
 */
template <typename Message>
concept MessageLike =
    requires { std::is_convertible_v<Message, socket_message_type>; };

/**
 * @brief Accepts a new connection on a socket.
 * @tparam Socket The socket type.
 * @param socket The listening socket.
 * @param address A buffer to store the address of the connecting socket.
 * @return A pair containing the new socket and a span of the address.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] accept_t tag, const Socket &socket,
                std::span<std::byte> address) -> decltype(auto) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto *sockaddr = reinterpret_cast<sockaddr_type *>(address.data());
  socklen_type socklen = address.size();

  native_socket_type sockfd = INVALID_SOCKET;
  while ((sockfd = ::accept(static_cast<native_socket_type>(socket), sockaddr,
                            &socklen)) == INVALID_SOCKET) {
    if (errno != EINTR)
      break;
  }

  return std::make_pair(Socket{sockfd}, address.subspan(0, socklen));
}

/**
 * @brief Binds a socket to a local address.
 * @tparam Socket The socket type.
 * @param socket The socket to bind.
 * @param address The local address to bind to.
 * @return 0 on success, -1 on error.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] bind_t tag, const Socket &socket,
                std::span<const std::byte> address) -> int {
  const auto *sockaddr =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const sockaddr_type *>(address.data());
  return ::bind(static_cast<native_socket_type>(socket), sockaddr,
                address.size());
}

/**
 * @brief Establishes a connection on a socket.
 * @tparam Socket The socket type.
 * @param socket The socket to connect.
 * @param address The remote address to connect to.
 * @return 0 on success, -1 on error.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] connect_t tag, const Socket &socket,
                std::span<const std::byte> address) -> int {
  int ret = -1;
  while ((ret = ::connect(
              static_cast<native_socket_type>(socket),
              // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
              reinterpret_cast<const sockaddr_type *>(address.data()),
              address.size())) == -1) {
    if (errno != EINTR)
      break;
  }
  return ret;
}

/**
 * @brief Manipulates the file descriptor of a socket.
 * @tparam Socket The socket type.
 * @tparam Args Additional arguments for fcntl.
 * @param socket The socket to manipulate.
 * @param cmd The command to perform.
 * @param args Additional arguments for the command.
 * @return The return value of fcntl.
 */
template <SocketLike Socket, typename... Args>
auto tag_invoke([[maybe_unused]] fcntl_t tag, const Socket &socket, int cmd,
                Args &&...args) -> int {
  return ::io::socket::fcntl(static_cast<native_socket_type>(socket), cmd,
                             std::forward<Args>(args)...);
}

/**
 * @brief Gets the address of the peer connected to a socket.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param address A buffer to store the peer's address.
 * @return A span of the peer's address.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] getpeername_t tag, const Socket &socket,
                std::span<std::byte> address) -> std::span<const std::byte> {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto *sockaddr = reinterpret_cast<sockaddr_type *>(address.data());
  socklen_type socklen = address.size();

  if (::getpeername(static_cast<native_socket_type>(socket), sockaddr,
                    &socklen)) {
    return {};
  }

  return address.subspan(0, socklen);
}

/**
 * @brief Gets the local address of a socket.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param address A buffer to store the local address.
 * @return A span of the local address.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] getsockname_t tag, const Socket &socket,
                std::span<std::byte> address) -> std::span<const std::byte> {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto *sockaddr = reinterpret_cast<sockaddr_type *>(address.data());
  socklen_type socklen = address.size();

  if (::getsockname(static_cast<native_socket_type>(socket), sockaddr,
                    &socklen)) {
    return {};
  }

  return address.subspan(0, socklen);
}

/**
 * @brief Gets a socket option.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option A buffer to store the option value.
 * @return A pair containing the return value of getsockopt and a span of the
 * option value.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] getsockopt_t tag, const Socket &socket,
                int level, int optname,
                std::span<std::byte> option) -> decltype(auto) {
  void *optval = option.data();
  socklen_type optlen = option.size();

  auto ret = ::getsockopt(static_cast<native_socket_type>(socket), level,
                          optname, optval, &optlen);

  return std::make_pair(ret, option.subspan(0, optlen));
}

/**
 * @brief Marks a socket as passive, that is, as a socket that will be used to
 * accept incoming connection requests.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param backlog The maximum length to which the queue of pending connections
 * for sockfd may grow.
 * @return 0 on success, -1 on error.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] listen_t tag, const Socket &socket,
                int backlog) -> int {
  return ::listen(static_cast<native_socket_type>(socket), backlog);
}

/**
 * @brief Receives a message from a socket.
 * @tparam Socket The socket type.
 * @tparam Message The message type.
 * @param socket The socket.
 * @param msg The message to receive.
 * @param flags The flags for the receive operation.
 * @return The number of bytes received.
 */
template <SocketLike Socket, MessageLike Message>
auto tag_invoke([[maybe_unused]] recvmsg_t tag, const Socket &socket,
                Message &msg, int flags) -> std::streamsize {
  std::streamsize len = -1;
  auto msghdr = static_cast<socket_message_type>(msg);

  while ((len = ::recvmsg(static_cast<native_socket_type>(socket), &msghdr,
                          flags)) < 0) {
    if (errno != EINTR)
      break;
  }

  return len;
}

/**
 * @brief Sends a message on a socket.
 * @tparam Socket The socket type.
 * @tparam Message The message type.
 * @param socket The socket.
 * @param msg The message to send.
 * @param flags The flags for the send operation.
 * @return The number of bytes sent.
 */
template <SocketLike Socket, MessageLike Message>
auto tag_invoke([[maybe_unused]] sendmsg_t tag, const Socket &socket,
                Message &msg, int flags) -> std::streamsize {
  std::streamsize len = 0;
  auto msghdr = static_cast<socket_message_type>(msg);

  while ((len = ::sendmsg(static_cast<native_socket_type>(socket), &msghdr,
                          flags)) < 0) {
    if (errno != EINTR)
      break;
  }

  return len;
}

/**
 * @brief Sets a socket option.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option The option value.
 * @return 0 on success, -1 on error.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] setsockopt_t tag, const Socket &socket,
                int level, int optname,
                std::span<const std::byte> option) -> int {
  return ::setsockopt(static_cast<native_socket_type>(socket), level, optname,
                      option.data(), option.size());
}

/**
 * @brief Shuts down part of a full-duplex connection.
 * @tparam Socket The socket type.
 * @param socket The socket.
 * @param how A flag that has one of the following values: SHUT_RD, SHUT_WR,
 * SHUT_RDWR.
 * @return 0 on success, -1 on error.
 */
template <SocketLike Socket>
auto tag_invoke([[maybe_unused]] shutdown_t tag, const Socket &socket,
                int how) -> int {
  return ::shutdown(static_cast<native_socket_type>(socket), how);
}

// TODO: Implement free-standing functions in the berkeley sockets APIs
// - send
// - sendto
// - recv
// - recvfrom

} // namespace io::socket
#endif // IO_OPERATIONS_HPP
