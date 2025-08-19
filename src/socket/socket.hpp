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
 * @file socket.hpp
 * @brief Dispatches platform-specific socket definitions and functions.
 *
 * This file includes either `windows_socket.hpp` or `posix_socket.hpp`
 * based on the operating system, providing a unified interface for
 * socket operations.
 */
#pragma once
#ifndef IO_SOCKET_HPP
#define IO_SOCKET_HPP
#include "socket_address.hpp" // IWYU pragma: export
#include "socket_handle.hpp"  // IWYU pragma: export
#include "socket_message.hpp" // IWYU pragma: export

#include "detail/socket_api.hpp"

/**
 * @namespace io::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace io::socket {

/**
 * @brief Binds a socket to a local address.
 * @relates socket_handle
 *
 * Binds the specified socket handle to a local address. This function
 * associates the socket with the address specified by `addr`.
 *
 * @param socket The `socket_handle` to be bound.
 * @param addr A pointer to a `sockaddr_type` structure containing the address
 * to bind to.
 * @param len The length of the `addr` structure.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::bind_t, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int;

/**
 * @brief Binds a socket to a local address.
 * @relates socket_handle
 *
 * Binds the specified socket handle to a local address. This function
 * associates the socket with the address specified by `addr`.
 *
 * @param socket The `socket_handle` to be bound.
 * @param addr The `socket_address` to bind to.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::bind_t, const socket_handle &socket,
                const socket_address &addr) -> int;

/**
 * @brief Sets a socket to listen for incoming connections.
 * @relates socket_handle
 *
 * Marks the socket as a passive socket, that is, as a socket that will be
 * used to accept incoming connection requests.
 *
 * @param socket The `socket_handle` to be marked for listening.
 * @param backlog The maximum number of pending connections.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::listen_t, const socket_handle &socket,
                int backlog) -> int;

/**
 * @brief Connects a socket to a remote address.
 * @relates socket_handle
 *
 * Establishes a connection on a socket.
 *
 * @param socket The `socket_handle` to connect.
 * @param addr A pointer to a `sockaddr_type` structure containing the address
 * to connect to.
 * @param len The length of the `addr` structure.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::connect_t, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int;

/**
 * @brief Connects a socket to a remote address.
 * @relates socket_handle
 *
 * Establishes a connection on a socket.
 *
 * @param socket The `socket_handle` to connect.
 * @param addr The `socket_address` to connect to.
 * @return 0 on success, or -1 on error, with `errno` set appropriately.
 */
auto tag_invoke(::io::connect_t, const socket_handle &socket,
                const socket_address &addr) -> int;

/**
 * @brief Accepts an incoming connection on a listening socket.
 * @relates socket_handle
 *
 * Extracts the first connection request on the queue of pending connections for
 * the listening socket.
 *
 * @param socket The listening `socket_handle`.
 * @param addr A pointer to a `sockaddr_type` structure that will be filled
 * with the address of the connecting socket.
 * @param len A pointer to a `socklen_type` that will be filled with the
 * length of the address.
 * @return The native socket for the accepted socket, or -1 on error, with
 * `errno` set appropriately.
 */
auto tag_invoke(::io::accept_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> native_socket_type;

/**
 * @brief Accepts an incoming connection on a listening socket.
 * @relates socket_handle
 *
 * Extracts the first connection request on the queue of pending connections for
 * the listening socket.
 *
 * @param socket The listening `socket_handle`.
 * @param addr An optional `socket_address` to be populated with the address of
 * the connecting socket.
 * @return A `std::tuple` containing a `socket_handle` for the accepted
 * socket and a `socket_address` for the connecting socket.
 * @throws std::system_error on failure.
 */
auto tag_invoke(::io::accept_t, const socket_handle &socket,
                socket_address addr = {})
    -> std::tuple<socket_handle, socket_address>;

/**
 * @brief Sends a message through a socket using a message structure.
 * @param socket The socket to send the message on.
 * @param msg A pointer to the msghdr structure containing the message to
 * send.
 * @param flags A bitmask of flags to control the send operation.
 * @return The number of bytes sent on success, or -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::sendmsg_t, const socket_handle &socket,
                const socket_message_type *msg, int flags) -> std::streamsize;

/**
 * @brief Receives a message from a socket using a message structure.
 * @param socket The socket to receive the message from.
 * @param msg A pointer to the msghdr structure to store the received
 * message.
 * @param flags A bitmask of flags to control the receive operation.
 * @return The number of bytes received on success, or -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::recvmsg_t, const socket_handle &socket,
                socket_message_type *msg, int flags) -> std::streamsize;

/**
 * @brief Gets a socket option value.
 * @param socket The socket to get the option from.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param optval A pointer to the buffer to receive the option value.
 * @param optlen A pointer to the length of the buffer.
 * @return 0 on success, -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::getsockopt_t, const socket_handle &socket, int level,
                int optname, void *optval, socklen_type *optlen) -> int;

/**
 * @brief Sets a socket option value.
 * @param socket The socket to set the option on.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param optval A pointer to the buffer containing the option value.
 * @param optlen The length of the buffer.
 * @return 0 on success, -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::setsockopt_t, const socket_handle &socket, int level,
                int optname, const void *optval, socklen_type optlen) -> int;

/**
 * @brief Gets the address bound to a socket.
 * @param socket The socket to get the address from.
 * @param addr A pointer to a sockaddr structure to receive the address.
 * @param len A pointer to a socklen_t to receive the length of the address.
 * @return 0 on success, -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::getsockname_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> int;

/**
 * @brief Gets the address of the peer connected to a socket.
 * @param socket The socket to get the peer address from.
 * @param addr A pointer to a sockaddr structure to receive the peer's
 * address.
 * @param len A pointer to a socklen_t to receive the length of the peer's
 * address.
 * @return 0 on success, -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::getpeername_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> int;

/**
 * @brief Shuts down part of a socket connection.
 * @param socket The socket to shut down.
 * @param how An integer indicating how to shut down the connection (e.g.,
 * SHUT_RD, SHUT_WR, SHUT_RDWR).
 * @return 0 on success, -1 on error.
 * @relates socket_handle
 */
auto tag_invoke(::io::shutdown_t, const socket_handle &socket, int how) -> int;

/**
 * @brief Performs file control operations on a socket.
 * @param socket The socket to perform the operation on.
 * @param operation The operation to perform.
 * @param args Additional arguments for the operation.
 * @return The result of the operation, or -1 on error.
 * @relates socket_handle
 */
template <typename... Args>
auto tag_invoke([[maybe_unused]] ::io::fcntl_t tag, const socket_handle &socket,
                int cmd, Args &&...args) -> int {
  return ::io::socket::fcntl(static_cast<native_socket_type>(socket), cmd,
                             std::forward<Args>(args)...);
}

// TODO: Implement free-standing functions in the berkeley sockets APIs
// - send
// - sendto
// - recv
// - recvfrom
} // namespace io::socket

#endif // IOSCHED_SOCKET_HPP
