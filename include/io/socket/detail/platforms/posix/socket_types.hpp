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
 * @file socket_types.hpp
 * @brief This file contains the POSIX-specific socket types
 */
#pragma once
#ifndef IO_SOCKET_TYPES_HPP
#define IO_SOCKET_TYPES_HPP
#include <sys/socket.h>
namespace io::socket {

/** @brief The native socket handle type for POSIX systems. */
using native_socket_type = int;

/** @brief Represents an invalid socket descriptor on POSIX systems. */
inline static constexpr native_socket_type INVALID_SOCKET = -1;

/** @brief A sentinel value used to indicate an error in socket API calls. */
inline static constexpr int SOCKET_ERROR = -1;

/** @brief The native buffer type for scatter/gather I/O.*/
using native_buffer_type = struct iovec;

/** @brief The socket message type for POSIX systems. */
using socket_message_type = struct msghdr;

/** @brief The generic socket address structure for POSIX systems. */
using sockaddr_type = struct sockaddr;

/** @brief The socket address storage structure for POSIX systems. */
using sockaddr_storage_type = struct sockaddr_storage;

/** @brief The type used to represent socket-related sizes on POSIX systems. */
using socklen_type = socklen_t;

} // namespace io::socket

#endif // IO_SOCKET_TYPES_HPP
