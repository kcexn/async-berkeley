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
 * @brief This file contains the Windows-specific socket types
 */
#pragma once
#ifndef IO_SOCKET_TYPES_HPP
#define IO_SOCKET_TYPES_HPP
#include <winsock2.h>
namespace io::socket {

/** @brief The native socket handle type for Windows systems. */
using native_socket_type = ::SOCKET;

/** @brief Represents an invalid socket descriptor on Windows systems. */
inline static constexpr native_socket_type INVALID_SOCKET = INVALID_SOCKET;

/** @brief A sentinel value used to indicate an error in socket API calls. */
inline static constexpr int SOCKET_ERROR = ::SOCKET_ERROR;

/** @brief The native buffer type for scatter/gather I/O.*/
using native_buffer_type = ::WSABUF;

/** @brief The socket message type for Windows systems. */
using socket_message_type = ::WSAMSG;

/** @brief The generic socket address structure for Windows systems. */
using sockaddr_type = ::SOCKADDR;

/** @brief The socket address storage structure for Windows systems. */
using sockaddr_storage_type = ::SOCKADDR_STORAGE;

/** @brief The type used to represent socket-related sizes on Windows systems.
 */
using socklen_type = int;

} // namespace io::socket

#endif // IO_SOCKET_TYPES_HPP
