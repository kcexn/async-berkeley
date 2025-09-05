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
#include "detail/operations.hpp" // IWYU pragma: export
#include "socket_address.hpp"    // IWYU pragma: export
#include "socket_handle.hpp"     // IWYU pragma: export
#include "socket_message.hpp"    // IWYU pragma: export
#include "socket_option.hpp"     // IWYU pragma: export
#endif                           // IO_SOCKET_HPP
