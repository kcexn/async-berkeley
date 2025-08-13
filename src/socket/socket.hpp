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
 * @brief Provides core cross-platform socket definitions and functions.
 *
 * This file contains fundamental type aliases and functions for handling
 * platform-specific socket operations in a generic way. It ensures that
 * socket code can be written once and compiled on both Windows and
 * POSIX-compliant systems.
 */
#pragma once
#ifndef IOSCHED_SOCKET_HPP
#define IOSCHED_SOCKET_HPP
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 *
 * This namespace contains fundamental types and functions for abstracting
 * away platform-specific socket details, allowing for portable network code.
 */
namespace iosched::socket {

#if BOOST_OS_WINDOWS
/**
 * @typedef native_socket_type
 * @brief Cross-platform alias for the native socket handle.
 *
 * This type is an alias for the underlying socket descriptor used by the
 * operating system. It is `::SOCKET` on Windows and `int` on POSIX systems.
 */
using native_socket_type = ::SOCKET;

/**
 * @brief Represents an invalid socket descriptor.
 *
 * This constant holds the value of an invalid socket for the target platform.
 * It is `INVALID_SOCKET` (from Winsock) on Windows and `-1` on POSIX systems.
 */
inline static constexpr native_socket_type INVALID_SOCKET = INVALID_SOCKET;

/**
 * @brief Closes a socket descriptor in a platform-independent manner.
 * @param socket The native socket handle to close.
 * @return `0` on success, or an error code on failure.
 *
 * This function wraps the native socket closing function (`::closesocket` on
 * Windows, `::close` on POSIX).
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::closesocket(socket);
}
#else
/**
 * @typedef native_socket_type
 * @brief Cross-platform alias for the native socket handle.
 *
 * This type is an alias for the underlying socket descriptor used by the
 * operating system. It is `::SOCKET` on Windows and `int` on POSIX systems.
 */
using native_socket_type = int;

/**
 * @brief Represents an invalid socket descriptor.
 *
 * This constant holds the value of an invalid socket for the target platform.
 * It is `INVALID_SOCKET` (from Winsock) on Windows and `-1` on POSIX systems.
 */
inline static constexpr native_socket_type INVALID_SOCKET = -1;

/**
 * @brief Closes a socket descriptor in a platform-independent manner.
 * @param socket The native socket handle to close.
 * @return `0` on success, or an error code on failure.
 *
 * This function wraps the native socket closing function (`::closesocket` on
 * Windows, `::close` on POSIX).
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::close(socket);
}
#endif

} // namespace iosched::socket
#endif // IOSCHED_SOCKET_HPP
