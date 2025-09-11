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
 * @file customization.hpp
 * @brief This file defines generic customization points
 */
#pragma once
#ifndef IO_CUSTOMIZATION_HPP
#define IO_CUSTOMIZATION_HPP
#include <span>
#include <utility>
/**
 * @brief The `io` namespace contains all the functions and classes for the I/O
 * library.
 *
 * This namespace provides a set of customization point objects (CPOs) for
 * various I/O operations. These CPOs are designed to be extensible and can be
 * customized for different types of I/O objects.
 */
namespace io {

/** @internal */
///@{
struct accept_t {};
struct bind_t {};
struct connect_t {};
struct fcntl_t {};
struct getpeername_t {};
struct getsockname_t {};
struct getsockopt_t {};
struct listen_t {};
struct recvmsg_t {};
struct sendmsg_t {};
struct setsockopt_t {};
struct shutdown_t {};

// TODO: Implement free-standing functions in the berkeley sockets APIs
// - send
// - sendto
// - recv
// - recvfrom

/**
 * @internal
 * @brief A generic function object that implements customization points.
 */
template <typename T> struct cpo {
  /**
   * @internal
   * @brief Invokes the customization point.
   * @tparam Args The types of the arguments to forward to the implementation.
   * @param ...args The arguments to forward to the implementation.
   * @return The value returned by the selected `tag_invoke` overload.
   */
  template <typename... Args>
  constexpr auto operator()(Args &&...args) const -> decltype(auto) {
    return tag_invoke(static_cast<T *>(nullptr), std::forward<Args>(args)...);
  }
};
///@}

/**
 * @defgroup cpo Sockets
 * @brief Functions for I/O Operations.
 *
 * These functions are the main interface for performing socket
 * operations. They dispatch to different implementations based on the
 * arguments provided, allowing for both synchronous and asynchronous use.
 */
/** @addtogroup cpo */
/** @{ */
/**
 * @brief Accepts an incoming connection on a listening socket.
 * @param socket A socket-like object.
 * @param address A span to be populated with the peer's address.
 * @return A `(socket, address)` pair for synchronous operations, or a
 *         `stdexec::sender` for asynchronous operations.
 */
inline auto accept(auto &&socket,
                   std::span<std::byte> address) -> decltype(auto) {
  static constexpr cpo<accept_t> accept{};
  return accept(std::forward<decltype(socket)>(socket), std::move(address));
}

/**
 * @brief Binds a socket to a local address.
 * @param socket A socket-like object.
 * @param address The local address to bind to.
 * @return 0 on success, -1 on error.
 */
inline auto bind(auto &&socket,
                 std::span<const std::byte> address) -> decltype(auto) {
  static constexpr cpo<bind_t> bind{};
  return bind(std::forward<decltype(socket)>(socket), std::move(address));
}

/**
 * @brief Connects a socket to a remote address.
 * @param socket A socket-like object.
 * @param address The remote address to connect to.
 * @return 0 on success, -1 on error for synchronous operations. A
 *         `stdexec::sender` for asynchronous operations.
 */
inline auto connect(auto &&socket,
                    std::span<const std::byte> address) -> decltype(auto) {
  static constexpr cpo<connect_t> connect{};
  return connect(std::forward<decltype(socket)>(socket), std::move(address));
}

/**
 * @brief Performs a file control operation on a socket.
 * @param socket A socket-like object.
 * @param cmd The command to perform.
 * @param args Optional arguments for the command.
 * @return The result of the underlying `fcntl` call.
 */
inline auto fcntl(auto &&socket, int cmd, auto &&...args) -> decltype(auto) {
  static constexpr cpo<fcntl_t> fcntl{};
  return fcntl(std::forward<decltype(socket)>(socket), cmd,
               std::forward<decltype(args)>(args)...);
}

/**
 * @brief Gets the peer address of a connected socket.
 * @param socket A socket-like object.
 * @param address A span to be populated with the peer's address.
 * @return A span containing the peer's address.
 */
inline auto getpeername(auto &&socket,
                        std::span<std::byte> address) -> decltype(auto) {
  static constexpr cpo<getpeername_t> getpeername{};
  return getpeername(std::forward<decltype(socket)>(socket),
                     std::move(address));
}

/**
 * @brief Gets a socket option.
 * @param socket A socket-like object.
 * @param level The protocol level of the option.
 * @param optname The option name.
 * @param option A span to be populated with the option value.
 * @return A `(result, option)` pair, where `result` is the return code of
 *         the underlying `getsockopt` call.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto getsockopt(auto &&socket, int level, int optname,
                       std::span<std::byte> option) -> decltype(auto) {
  static constexpr cpo<getsockopt_t> getsockopt{};
  return getsockopt(std::forward<decltype(socket)>(socket), level, optname,
                    std::move(option));
}

/**
 * @brief Gets the local address of a socket.
 * @param socket A socket-like object.
 * @param address A span to be populated with the local address.
 * @return A span containing the local socket address.
 */
inline auto getsockname(auto &&socket,
                        std::span<std::byte> address) -> decltype(auto) {
  static constexpr cpo<getsockname_t> getsockname{};
  return getsockname(std::forward<decltype(socket)>(socket),
                     std::move(address));
}

/**
 * @brief Sets a socket to listen for incoming connections.
 * @param socket A socket-like object.
 * @param backlog The maximum length of the pending connections queue.
 * @return The result of the underlying `listen` call.
 */
inline auto listen(auto &&socket, int backlog) -> decltype(auto) {
  static constexpr cpo<listen_t> listen{};
  return listen(std::forward<decltype(socket)>(socket), backlog);
}

/**
 * @brief Receives a message from a socket.
 * @param socket A socket-like object.
 * @param msg A message object to receive data into.
 * @param flags Flags to control the receive operation.
 * @return The number of bytes received for synchronous operations. A
 *         `stdexec::sender` for asynchronous operations.
 */
inline auto recvmsg(auto &&socket, auto &&msg, int flags) -> decltype(auto) {
  static constexpr cpo<recvmsg_t> recvmsg{};
  return recvmsg(std::forward<decltype(socket)>(socket),
                 std::forward<decltype(msg)>(msg), flags);
}

/**
 * @brief Sends a message on a socket.
 * @param socket A socket-like object.
 * @param msg The message object to send.
 * @param flags Flags to control the send operation.
 * @return The number of bytes sent for synchronous operations. A
 *         `stdexec::sender` for asynchronous operations.
 */
inline auto sendmsg(auto &&socket, auto &&msg, int flags) -> decltype(auto) {
  static constexpr cpo<sendmsg_t> sendmsg{};
  return sendmsg(std::forward<decltype(socket)>(socket),
                 std::forward<decltype(msg)>(msg), flags);
}

/**
 * @brief Sets a socket option.
 * @param socket A socket-like object.
 * @param level The protocol level of the option.
 * @param optname The option name.
 * @param option A span containing the option value.
 * @return The result of the underlying `setsockopt` call.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto setsockopt(auto &&socket, int level, int optname,
                       std::span<const std::byte> option) -> decltype(auto) {
  static constexpr cpo<setsockopt_t> setsockopt{};
  return setsockopt(std::forward<decltype(socket)>(socket), level, optname,
                    std::move(option));
}

/**
 * @brief Shuts down all or part of a connection on a socket.
 * @param socket A socket-like object.
 * @param how A flag specifying which parts of the connection to shut down.
 * @return The result of the underlying `shutdown` call.
 */
inline auto shutdown(auto &&socket, int how) -> decltype(auto) {
  static constexpr cpo<shutdown_t> shutdown{};
  return shutdown(std::forward<decltype(socket)>(socket), how);
}
/** @} */

} // namespace io
#endif // IO_CUSTOMIZATION_HPP
