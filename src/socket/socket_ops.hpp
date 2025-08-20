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
 * @file socket_ops.hpp
 * @brief This file defines the socket customization point operations
 */
#pragma once
#ifndef IO_SOCKET_OPS_HPP
#define IO_SOCKET_OPS_HPP
namespace io::socket {
/// @brief A tag type for the `io::accept` customization point object (CPO).
struct accept_t {};
/// @brief A tag type for the `io::bind` customization point object (CPO).
struct bind_t {};
/// @brief A tag type for the `io::connect` customization point object (CPO).
struct connect_t {};
/// @brief A tag type for the `io::fcntl` customization point object (CPO).
struct fcntl_t {};
/// @brief A tag type for the `io::getpeername` customization point object
/// (CPO).
struct getpeername_t {};
/// @brief A tag type for the `io::getsockname` customization point object
/// (CPO).
struct getsockname_t {};
/// @brief A tag type for the `io::getsockopt` customization point object (CPO).
struct getsockopt_t {};
/// @brief A tag type for the `io::listen` customization point object (CPO).
struct listen_t {};
/// @brief A tag type for the `io::recvmsg` customization point object (CPO).
struct recvmsg_t {};
/// @brief A tag type for the `io::sendmsg` customization point object (CPO).
struct sendmsg_t {};
/// @brief A tag type for the `io::setsockopt` customization point object (CPO).
struct setsockopt_t {};
/// @brief A tag type for the `io::shutdown` customization point object (CPO).
struct shutdown_t {};
} // namespace io::socket
#endif // IO_ACCEPT_HPP
