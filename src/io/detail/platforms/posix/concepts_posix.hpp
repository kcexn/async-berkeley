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
 * @file concepts_posix.hpp
 * @brief This file defines concepts for the execution components.
 */
#pragma once
#ifndef IO_CONCEPTS_POSIX_HPP
#define IO_CONCEPTS_POSIX_HPP
#include <type_traits>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
/**
 * @namespace io
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io {

template <typename T>
concept SocketAddress = std::is_same_v<std::decay_t<T>, sockaddr> ||
                        std::is_same_v<std::decay_t<T>, sockaddr_storage> ||
                        std::is_same_v<std::decay_t<T>, sockaddr_in> ||
                        std::is_same_v<std::decay_t<T>, sockaddr_in6> ||
                        std::is_same_v<std::decay_t<T>, sockaddr_un>;

} // namespace io
#endif // IO_CONCEPTS_POSIX_HPP
