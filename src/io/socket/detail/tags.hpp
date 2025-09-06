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
 * @file tags.hpp
 * @brief Defines the available socket operations using the tag_invoke
 * customization point.
 */
#pragma once
#ifndef IO_TAGS_HPP
#define IO_TAGS_HPP
namespace io::socket {

struct accept_t {};
struct bind_t {};
struct connect_t {};
struct fcntl_t {};
struct getpeername_t {};
struct getsockname_t {};
struct getsockopt_t {};
struct listen_t {};
struct recvmsg_t {};
struct sendmsg_t {};
struct setsockopt_t {};
struct shutdown_t {};

// TODO: Implement free-standing functions in the berkeley sockets APIs
// - send
// - sendto
// - recv
// - recvfrom
} // namespace io::socket
#endif // IO_TAGS_HPP
