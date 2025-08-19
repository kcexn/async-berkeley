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
 * @file socket_api.hpp
 * @brief This file includes all the necessary headers for the socket API.
 *
 * It serves as a single entry point for including all the socket-related CPOs.
 */
#pragma once
#ifndef IO_SOCKET_API_HPP
#define IO_SOCKET_API_HPP
#include "../../detail/accept.hpp"      // IWYU pragma: export
#include "../../detail/bind.hpp"        // IWYU pragma: export
#include "../../detail/connect.hpp"     // IWYU pragma: export
#include "../../detail/fcntl.hpp"       // IWYU pragma: export
#include "../../detail/getpeername.hpp" // IWYU pragma: export
#include "../../detail/getsockname.hpp" // IWYU pragma: export
#include "../../detail/getsockopt.hpp"  // IWYU pragma: export
#include "../../detail/listen.hpp"      // IWYU pragma: export
#include "../../detail/recvmsg.hpp"     // IWYU pragma: export
#include "../../detail/sendmsg.hpp"     // IWYU pragma: export
#include "../../detail/setsockopt.hpp"  // IWYU pragma: export
#include "../../detail/shutdown.hpp"    // IWYU pragma: export
#endif                                  // IO_SOCKET_API_HPP
