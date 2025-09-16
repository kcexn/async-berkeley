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
 * @brief This file contains the POSIX-specific socket definitions and
 * functions.
 */
#pragma once
#ifndef IO_SOCKET_HPP
#define IO_SOCKET_HPP
#include "io/config.h"
#if OS_WINDOWS
#include "platforms/windows/socket_types.hpp" // IWYU pragma: export
#else
#include "platforms/posix/socket_types.hpp" // IWYU pragma: export
#endif
#include <ios>
namespace io::socket {
/**
 * @brief Closes a socket descriptor on POSIX systems.
 * @param socket The native socket handle to close.
 * @return 0 on success, or an error code on failure.
 */
inline auto close(native_socket_type socket) noexcept -> int;
/**
 * @brief Provides a C++ wrapper for the POSIX `fcntl` function.
 * @param socket The native socket descriptor.
 * @param cmd The `fcntl` command to execute.
 * @param ...args The arguments for the specified command.
 * @return The result of the underlying `::fcntl` call.
 */
template <typename... Args>
inline auto fcntl(native_socket_type socket, int cmd,
                  Args &&...args) noexcept -> int;
/**
 * @brief Sends a message on a socket.
 * @param socket The native socket handle.
 * @param msg A pointer to the `msghdr` structure containing the message to
 * send.
 * @param flags A bitwise OR of flags to modify the send behavior.
 * @return The number of bytes sent on success, or `SOCKET_ERROR` on failure.
 */
inline auto sendmsg(native_socket_type socket, const socket_message_type *msg,
                    int flags) noexcept -> std::streamsize;

/**
 * @brief Receives a message from a socket.
 * @param socket The native socket handle.
 * @param msg A pointer to the `msghdr` structure to store the received message.
 * @param flags A bitwise OR of flags to modify the receive behavior.
 * @return The number of bytes received on success, or `SOCKET_ERROR` on
 * failure.
 */
inline auto recvmsg(native_socket_type socket, socket_message_type *msg,
                    int flags) noexcept -> std::streamsize;
} // namespace io::socket

#if OS_WINDOWS
#include "platforms/windows/socket_functions.hpp" // IWYU pragma: export
#else
#include "platforms/posix/socket_functions.hpp" // IWYU pragma: export
#endif

#endif // IO_SOCKET_HPP
