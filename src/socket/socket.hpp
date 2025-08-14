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
#include "windows_socket.hpp" // IWYU pragma: export
#else
#include "posix_socket.hpp" // IWYU pragma: export
#endif
} // namespace iosched::socket

#endif // IOSCHED_SOCKET_HPP
