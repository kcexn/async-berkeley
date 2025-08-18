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
 * @brief Provides core POSIX-specific socket definitions and functions.
 *
 * This file contains fundamental type aliases and functions for handling
 * socket operations specifically on POSIX systems.
 */
#pragma once
#ifndef IO_SOCKET_POSIX_HPP
#define IO_SOCKET_POSIX_HPP
#include <ios>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @namespace io::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 *
 * This namespace contains fundamental types and functions for abstracting
 * away platform-specific socket details, allowing for portable network code.
 */
namespace io::socket {
/**
 * @typedef native_socket_type
 * @brief Alias for the native socket handle on POSIX systems.
 *
 * This type is an alias for the underlying socket descriptor used by the
 * operating system, which is an `int` on POSIX systems.
 */
using native_socket_type = int;

/**
 * @brief Represents an invalid socket descriptor on POSIX systems.
 *
 * This constant holds the value of an invalid socket, which is `-1` on
 * POSIX systems.
 */
inline static constexpr native_socket_type INVALID_SOCKET = -1;

/**
 * @brief Sentinel value for error conditions on socket API calls.
 */
inline static constexpr int SOCKET_ERROR = -1;

/**
 * @brief socket message type
 */
using socket_message_type = struct msghdr;

/**
 * @brief Closes a socket descriptor on POSIX systems.
 * @param socket The native socket handle to close.
 * @return `0` on success, or an error code on failure.
 *
 * This function wraps the native `::close` function.
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::close(socket);
}

/**
 * @brief Provides a C++ wrapper for the POSIX fcntl function.
 *
 * This function is an inline wrapper around the standard POSIX `fcntl` system
 * call. It forwards the provided arguments directly to the underlying `::fcntl`
 * function.
 *
 * @param socket The native socket descriptor.
 * @param cmd The fcntl command to execute.
 * @param ...args The arguments for the specified command.
 * @return The result of the underlying `::fcntl` call. Typically, this is
 *         a value >= 0 on success and -1 on error, with `errno` set
 *         appropriately.
 */
template <typename... Args>
inline auto fcntl(native_socket_type socket, int cmd,
                  Args &&...args) noexcept -> int {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  return ::fcntl(socket, cmd, std::forward<Args>(args)...);
}

/**
 * @brief Sends a message on a socket.
 * @param socket The native socket handle.
 * @param msg A pointer to the `msghdr` structure containing the message to
 * send.
 * @param flags A bitwise OR of flags to modify the send behavior.
 * @return The number of bytes sent on success, or `SOCKET_ERROR` on failure.
 *
 * This function wraps the native `::sendmsg` function.
 */
inline auto sendmsg(native_socket_type socket, const socket_message_type *msg,
                    int flags) noexcept -> std::streamsize {
  return ::sendmsg(socket, msg, flags);
}

/**
 * @brief Receives a message from a socket.
 * @param socket The native socket handle.
 * @param msg A pointer to the `msghdr` structure to store the received message.
 * @param flags A bitwise OR of flags to modify the receive behavior.
 * @return The number of bytes received on success, or `SOCKET_ERROR` on
 * failure.
 *
 * This function wraps the native `::recvmsg` function.
 */
inline auto recvmsg(native_socket_type socket, socket_message_type *msg,
                    int flags) noexcept -> std::streamsize {
  return ::recvmsg(socket, msg, flags);
}

/**
 * @typedef sockaddr_type
 * @brief Alias for the generic socket address structure on POSIX systems.
 *
 * This type is an alias for `struct sockaddr`, which serves as the base
 * structure for all other socket address types (e.g., `sockaddr_in`). It is
 * primarily used for type-casting when passing addresses to socket functions.
 */
using sockaddr_type = struct sockaddr;

/**
 * @typedef sockaddr_storage_type
 * @brief Alias for the `sockaddr_storage` structure on POSIX systems.
 *
 * This type is used to store socket address information in a generic way,
 * large enough to accommodate all supported socket address types.
 */
using sockaddr_storage_type = struct sockaddr_storage;

/**
 * @typedef socket_size_type
 * @brief Alias for the type used to represent socket-related sizes on POSIX
 * systems.
 *
 * This type is typically `socklen_t` and is used for lengths of socket address
 * structures and other size parameters in POSIX socket functions.
 */
using socklen_type = socklen_t;
} // namespace io::socket

#endif // IO_SOCKET_POSIX_HPP
