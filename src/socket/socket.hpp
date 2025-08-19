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
 * @brief This file defines the core socket operations for the I/O library.
 *
 * It provides a cross-platform interface for socket programming by dispatching
 * to platform-specific implementations. This file includes the necessary
 * headers for socket operations and defines the `io::socket` namespace, which
 * contains functions for creating, connecting, and managing sockets.
 */
#pragma once
#ifndef IO_SOCKET_HPP
#define IO_SOCKET_HPP
#include "socket_address.hpp" // IWYU pragma: export
#include "socket_handle.hpp"  // IWYU pragma: export
#include "socket_message.hpp" // IWYU pragma: export

#include "detail/socket_api.hpp"

/**
 * @brief The `io::socket` namespace provides a cross-platform abstraction for
 * socket-level I/O operations.
 *
 * This namespace contains a set of functions that wrap the underlying
 * platform-specific socket APIs, providing a consistent and easy-to-use
 * interface for socket programming.
 */
namespace io::socket {

/**
 * @brief Binds a socket to a local address.
 *
 * This function associates a socket with a specific local address. This is
 * necessary before a socket can accept incoming connections.
 *
 * @param socket The socket handle to bind.
 * @param addr A pointer to a `sockaddr_type` structure containing the address
 * to bind to.
 * @param len The length of the `addr` structure.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::bind_t, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int;

/**
 * @brief Binds a socket to a local address.
 *
 * This function associates a socket with a specific local address. This is
 * necessary before a socket can accept incoming connections.
 *
 * @param socket The socket handle to bind.
 * @param addr The `socket_address` to bind to.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::bind_t, const socket_handle &socket,
                const socket_address &addr) -> int;

/**
 * @brief Sets a socket to listen for incoming connections.
 *
 * This function marks a socket as a passive socket, which will be used to
 * accept incoming connection requests.
 *
 * @param socket The socket handle to set to listen.
 * @param backlog The maximum number of pending connections that can be queued.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle
 */
auto tag_invoke(::io::listen_t, const socket_handle &socket,
                int backlog) -> int;

/**
 * @brief Connects a socket to a remote address.
 *
 * This function establishes a connection to a specified remote address.
 *
 * @param socket The socket handle to connect.
 * @param addr A pointer to a `sockaddr_type` structure containing the remote
 * address to connect to.
 * @param len The length of the `addr` structure.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::connect_t, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int;

/**
 * @brief Connects a socket to a remote address.
 *
 * This function establishes a connection to a specified remote address.
 *
 * @param socket The socket handle to connect.
 * @param addr The `socket_address` to connect to.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::connect_t, const socket_handle &socket,
                const socket_address &addr) -> int;

/**
 * @brief Accepts an incoming connection on a listening socket.
 *
 * This function extracts the first connection request from the queue of pending
 * connections for a listening socket.
 *
 * @param socket The listening socket handle.
 * @param addr A pointer to a `sockaddr_type` structure that will be filled with
 * the address of the connecting socket.
 * @param len A pointer to a `socklen_type` that will be filled with the length
 * of the address.
 * @return The native socket handle for the accepted socket, or -1 on error. In
 * case of an error, `errno` is set to indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::accept_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> native_socket_type;

/**
 * @brief Accepts an incoming connection on a listening socket.
 *
 * This function extracts the first connection request from the queue of pending
 * connections for a listening socket.
 *
 * @param socket The listening socket handle.
 * @param addr An optional `socket_address` to be populated with the address of
 * the connecting socket.
 * @return A `std::tuple` containing a `socket_handle` for the accepted socket
 * and a `socket_address` for the connecting socket.
 * @throws std::system_error on failure.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::accept_t, const socket_handle &socket,
                socket_address addr = {})
    -> std::tuple<socket_handle, socket_address>;

/**
 * @brief Sends a message on a socket.
 *
 * @param socket The socket to send the message on.
 * @param msg A pointer to a `msghdr` structure containing the message to send.
 * @param flags A bitmask of flags to control the send operation.
 * @return The number of bytes sent on success, or -1 on error. In case of an
 * error, `errno` is set to indicate the error.
 * @see socket_handle, socket_message
 */
auto tag_invoke(::io::sendmsg_t, const socket_handle &socket,
                const socket_message_type *msg, int flags) -> std::streamsize;

/**
 * @brief Receives a message from a socket.
 *
 * @param socket The socket to receive the message from.
 * @param msg A pointer to a `msghdr` structure to store the received message.
 * @param flags A bitmask of flags to control the receive operation.
 * @return The number of bytes received on success, or -1 on error. In case of
 * an error, `errno` is set to indicate the error.
 * @see socket_handle, socket_message
 */
auto tag_invoke(::io::recvmsg_t, const socket_handle &socket,
                socket_message_type *msg, int flags) -> std::streamsize;

/**
 * @brief Gets a socket option.
 *
 * @param socket The socket to get the option from.
 * @param level The protocol level at which the option resides.
 * @param optname The name of the option.
 * @param optval A pointer to the buffer where the option value will be stored.
 * @param optlen A pointer to the length of the buffer.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle
 */
auto tag_invoke(::io::getsockopt_t, const socket_handle &socket, int level,
                int optname, void *optval, socklen_type *optlen) -> int;

/**
 * @brief Sets a socket option.
 *
 * @param socket The socket to set the option on.
 * @param level The protocol level at which the option resides.
 * @param optname The name of the option.
 * @param optval A pointer to the buffer containing the option value.
 * @param optlen The length of the buffer.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle
 */
auto tag_invoke(::io::setsockopt_t, const socket_handle &socket, int level,
                int optname, const void *optval, socklen_type optlen) -> int;

/**
 * @brief Gets the local address of a socket.
 *
 * @param socket The socket to get the local address from.
 * @param addr A pointer to a `sockaddr` structure to store the local address.
 * @param len A pointer to a `socklen_t` to store the length of the address.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::getsockname_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> int;

/**
 * @brief Gets the peer address of a connected socket.
 *
 * @param socket The socket to get the peer address from.
 * @param addr A pointer to a `sockaddr` structure to store the peer address.
 * @param len A pointer to a `socklen_t` to store the length of the address.
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle, socket_address
 */
auto tag_invoke(::io::getpeername_t, const socket_handle &socket,
                sockaddr_type *addr, socklen_type *len) -> int;

/**
 * @brief Shuts down a socket connection.
 *
 * This function disables sending, receiving, or both on a socket.
 *
 * @param socket The socket to shut down.
 * @param how An integer indicating how to shut down the connection (e.g.,
 * SHUT_RD, SHUT_WR, SHUT_RDWR).
 * @return 0 on success, or -1 on error. In case of an error, `errno` is set to
 * indicate the error.
 * @see socket_handle
 */
auto tag_invoke(::io::shutdown_t, const socket_handle &socket, int how) -> int;

/**
 * @brief Performs a file control operation on a socket.
 *
 * @param socket The socket to perform the operation on.
 * @param cmd The file control command to execute.
 * @param args Additional arguments for the command.
 * @return The result of the operation, or -1 on error. In case of an error,
 * `errno` is set to indicate the error.
 * @see socket_handle
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
