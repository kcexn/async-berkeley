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
 * @defgroup cpo Function Objects
 * @brief Function Objects for I/O Operations.
 *
 * These function objects are the main interface for performing socket
 * operations. They dispatch to different implementations based on the
 * arguments provided, allowing for both synchronous and asynchronous use.
 */
/** @name Customization Points */
/** @{ */
/** @addtogroup cpo */
/** @{ */
/**
 * @fn auto accept(auto&& dialog, std::span<std::byte> address)
 * @brief Accepts an incoming connection on a listening socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `accept` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object that can accept a connection.
 * @param address A span that will be populated with the address of the
 *                connecting peer.
 * @return A (socket-like, addr) pair for synchronous operations, and a
 *         a stdexec::sender for asynchronous operations.
 */
inline constexpr cpo<accept_t> accept{};

/**
 * @fn auto bind(auto&& dialog, std::span<const std::byte> address)
 * @brief Binds a socket to an address.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `bind` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object that can bind to an address.
 * @param address A span containing the local address to bind the socket to.
 * @return 0 on success, -1 otherwise.
 */
inline constexpr cpo<bind_t> bind{};

/**
 * @fn auto connect(auto&& dialog, std::span<const std::byte> address)
 * @brief Connects a socket to an address.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `connect` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object that can connect to an address.
 * @param address A span containing the remote address to connect to.
 * @return 0 on success, -1 otherwise for synchronous operations. A
 *         stdexec::sender for asynchronous operations.
 */
inline constexpr cpo<connect_t> connect{};

/**
 * @fn auto fcntl(auto&& dialog, int cmd, auto... args)
 * @brief Performs a file control operation on a socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `fcntl` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param cmd The command to perform.
 * @param args Optional arguments for the command.
 * @return An `int`, the return value of the underlying `fcntl`
 *         implementation.
 */
inline constexpr cpo<fcntl_t> fcntl{};

/**
 * @fn auto getpeername(auto&& dialog, std::span<std::byte> address)
 * @brief Gets the peer address of a connected socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `getpeername` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param address A span that will be populated with the address of the peer.
 * @return A span containing the address of the peer socket.
 */
inline constexpr cpo<getpeername_t> getpeername{};

/**
 * @fn auto getsockopt(auto&& dialog, int level, int optname,
 * std::span<std::byte> option)
 * @brief Gets a socket option.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `getsockopt` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option A span that will be populated with the option value.
 * @return An (integer, std::span) pair that holds the return value
 *         of the underlying `getsockopt` implementation and a span
 *         containing the value of the option.
 */
inline constexpr cpo<getsockopt_t> getsockopt{};

/**
 * @fn auto getsockname(auto&& dialog, std::span<std::byte> address)
 * @brief Gets the local address of a socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `getsockname` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param address A span that will be populated with the local address.
 * @return A span that contains the local socket address.
 */
inline constexpr cpo<getsockname_t> getsockname{};

/**
 * @fn auto listen(auto&& dialog, int backlog)
 * @brief Sets a socket to listen for incoming connections.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `listen` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param backlog The maximum length of the queue of pending connections.
 * @return The return value of the underlying `listen` implementation.
 */
inline constexpr cpo<listen_t> listen{};

/**
 * @fn auto recvmsg(auto&& dialog, Message& msg, int flags)
 * @brief Receives a message from a socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `recvmsg` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param msg A message object to receive data into.
 * @param flags Flags to control the receive operation.
 * @return The return value of the underlying `recvmsg` implementation for
 *         for synchronous operations. A stdexec::sender for an asynchronous
 *         implementation.
 */
inline constexpr cpo<recvmsg_t> recvmsg{};

/**
 * @fn auto sendmsg(auto&& dialog, const Message& msg, int flags)
 * @brief Sends a message on a socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `sendmsg` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param msg The message object to send.
 * @param flags Flags to control the send operation.
 * @return The return value of the underlying `sendmsg` implementation for
 *         for synchronous operations. A stdexec::sender for an asynchronous
 *         implementation.
 */
inline constexpr cpo<sendmsg_t> sendmsg{};

/**
 * @fn auto setsockopt(auto&& dialog, int level, int optname, std::span<const
 * std::byte> option)
 * @brief Sets a socket option.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `setsockopt` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option A span containing the option value.
 * @return The return value of the underlying `setsockopt` implementation.
 */
inline constexpr cpo<setsockopt_t> setsockopt{};

/**
 * @fn auto shutdown(auto&& dialog, int how)
 * @brief Shuts down all or part of a connection on a socket.
 *
 * This is a customization point object that dispatches to a `tag_invoke`
 * overload. It is an abstraction over the `shutdown` function from the
 * Berkeley Sockets API.
 *
 * @param socket A socket like object.
 * @param how A flag that specifies what parts of the connection to shut down.
 * @return The return value of the underlying `shutdown` implementation.
 */
inline constexpr cpo<shutdown_t> shutdown{};
/** @} */
/** @} */

} // namespace io

#endif // IO_CUSTOMIZATION_HPP
