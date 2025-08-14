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
 * @file posix_socket.hpp
 * @brief Provides core POSIX-specific socket definitions and functions.
 *
 * This file contains fundamental type aliases and functions for handling
 * socket operations specifically on POSIX systems.
 */
#pragma once
#ifndef IOSCHED_POSIX_SOCKET_HPP
#define IOSCHED_POSIX_SOCKET_HPP
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
 * @brief Closes a socket descriptor on POSIX systems.
 * @param socket The native socket handle to close.
 * @return `0` on success, or an error code on failure.
 *
 * This function wraps the native `::close` function.
 */
inline auto close(native_socket_type socket) noexcept -> int {
  return ::close(socket);
}
#endif // IOSCHED_POSIX_SOCKET_HPP
