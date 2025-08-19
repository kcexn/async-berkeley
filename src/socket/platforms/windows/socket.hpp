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
 * @brief This file contains the Windows-specific socket definitions and
 * functions.
 */
#pragma once
#ifndef IO_SOCKET_WINDOWS_HPP
#define IO_SOCKET_WINDOWS_HPP
#include <cassert>
#include <ios>
#include <span>
#include <tuple>

#include <winsock2.h>
#include <ws2tcpip.h>

namespace io::socket {
/**
 * @brief The native socket handle type for Windows systems.
 *
 * This is an alias for the `::SOCKET` type provided by the Winsock library.
 */
using native_socket_type = ::SOCKET;

/**
 * @brief Represents an invalid socket descriptor on Windows systems.
 *
 * This constant holds the value of an invalid socket, which is `INVALID_SOCKET`
 * from the Winsock library.
 */
inline static constexpr native_socket_type INVALID_SOCKET = INVALID_SOCKET;

/**
 * @brief A sentinel value used to indicate an error in socket API calls.
 */
inline static constexpr int SOCKET_ERROR = ::SOCKET_ERROR;

/**
 * @brief The socket buffer type for Windows systems.
 *
 * This is an alias for the `::WSABUF` type provided by the Winsock library.
 */
using socket_buffer_type = std::span<char, std::dynamic_extent>;

/**
 * @brief The socket message type for Windows systems.
 *
 * This is an alias for the `::WSAMSG` type provided by the Winsock library.
 */
using socket_message_type = ::WSAMSG;

/**
 * @brief Closes a socket descriptor on Windows systems.
 *
 * This function wraps the native `::closesocket` function.
 *
 * @param socket The native socket handle to close.
 * @return 0 on success, or an error code on failure.
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::closesocket(socket);
}

// POSIX fcntl constants
static constexpr int F_SETFL = 4;
static constexpr int O_NONBLOCK = 2048;

/**
 * @brief Provides a Windows compatibility implementation of the POSIX `fcntl`
 * function.
 *
 * This function is a limited implementation of the POSIX `fcntl` function,
 * specifically tailored for setting the non-blocking mode on a socket. It
 * translates the `fcntl(socket, F_SETFL, O_NONBLOCK)` call to its Windows
 * equivalent using `ioctlsocket`.
 *
 * @note This implementation only supports the `F_SETFL` command with the
 * `O_NONBLOCK` flag. Any other usage will result in a compile-time or run-time
 * assertion failure.
 *
 * @param socket The native socket descriptor.
 * @param cmd The `fcntl` command to execute. Only `F_SETFL` is supported.
 * @param ...args A single argument, which must be the flags to set. Only
 * `O_NONBLOCK` is handled.
 * @return 0 on success, or `SOCKET_ERROR` on failure.
 */
template <typename... Args>
inline auto fcntl(native_socket_type socket, int cmd,
                  Args &&...args) noexcept -> int {
  static_assert(sizeof...(args) == 1,
                "fcntl for windows is only implemented for F_SETFL and a "
                "single argument O_NONBLOCK.");
  assert(cmd == F_SETFL && "fcntl for windows is only implemented for F_SETFL");

  auto arg = std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...));
  auto mode = (arg & O_NONBLOCK) ? 1 : 0;
  switch (cmd) {
  case F_SETFL:
    return ioctlsocket(socket, FIONBIO, &mode);
  default:
    return SOCKET_ERROR;
  }
}

/**
 * @brief Sends a message on a socket using `WSASendMsg`.
 *
 * This function wraps the native `::WSASendMsg` function.
 *
 * @param socket The native socket handle.
 * @param msg A pointer to the `WSAMSG` structure containing the message to
 * send.
 * @param flags This parameter is ignored on Windows.
 * @return The number of bytes sent on success, or a standard Windows Sockets
 * error code on failure.
 */
inline auto sendmsg(native_socket_type socket, const socket_message_type *msg,
                    int flags) noexcept -> std::streamsize {
  std::streamsize len = 0;
  int error = ::WSASendMsg(socket, msg, &len, nullptr, nullptr);
  return (error == 0) ? len : error;
}

/**
 * @brief Receives a message from a socket using `WSARecvMsg`.
 *
 * This function wraps the native `::WSARecvMsg` function.
 *
 * @param socket The native socket handle.
 * @param msg A pointer to the `WSAMSG` structure to store the received message.
 * @param flags A bitwise OR of flags to modify the receive behavior.
 * @return The number of bytes received on success, or a standard Windows
 * Sockets error code on failure.
 */
inline auto recvmsg(native_socket_type socket, socket_message_type *msg,
                    int flags) noexcept -> std::streamsize {
  std::streamsize len = 0;
  int error = ::WSARecvMsg(socket, msg, &len, nullptr, nullptr);
  return (error == 0) ? len : error;
}

/**
 * @brief The generic socket address structure for Windows systems.
 *
 * This is an alias for the `::SOCKADDR` type, which serves as the base
 * structure for all other socket address types (e.g., `SOCKADDR_IN`). It is
 * primarily used for type-casting when passing addresses to Winsock functions.
 */
using sockaddr_type = ::SOCKADDR;

/**
 * @brief The socket address storage structure for Windows systems.
 *
 * This is an alias for the `::SOCKADDR_STORAGE` type, which is used to store
 * socket address information in a generic way, large enough to accommodate all
 * supported socket address types.
 */
using sockaddr_storage_type = ::SOCKADDR_STORAGE;

/**
 * @brief The type used to represent socket-related sizes on Windows systems.
 *
 * This is an alias for the `int` type, which is used for the lengths of socket
 * address structures and other size parameters in Winsock functions.
 */
using socklen_type = int;
} // namespace io::socket

#endif // IO_SOCKET_WINDOWS_HPP
