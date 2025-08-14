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
 * @file windows_socket.hpp
 * @brief Provides core Windows-specific socket definitions and functions.
 *
 * This file contains fundamental type aliases and functions for handling
 * socket operations specifically on Windows systems.
 */
#pragma once
#ifndef IOSCHED_WINDOWS_SOCKET_HPP
#define IOSCHED_WINDOWS_SOCKET_HPP
/**
 * @typedef native_socket_type
 * @brief Alias for the native socket handle on Windows systems.
 *
 * This type is an alias for the underlying socket descriptor used by the
 * operating system, which is `::SOCKET` on Windows systems.
 */
using native_socket_type = ::SOCKET;

/**
 * @brief Represents an invalid socket descriptor on Windows systems.
 *
 * This constant holds the value of an invalid socket, which is
 * `INVALID_SOCKET` (from Winsock) on Windows systems.
 */
inline static constexpr native_socket_type INVALID_SOCKET = INVALID_SOCKET;

/**
 * @brief Closes a socket descriptor on Windows systems.
 * @param socket The native socket handle to close.
 * @return `0` on success, or an error code on failure.
 *
 * This function wraps the native `::closesocket` function.
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::closesocket(socket);
}
#endif // IOSCHED_WINDOWS_SOCKET_HPP
