/* Copyright 2025 Kevin Exton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is a "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file io.hpp
 * @brief Core types for the io library.
 */
#pragma once
#ifndef IO_HPP
#define IO_HPP
#include "detail/bind.hpp" // IWYU pragma: export
#include "detail/listen.hpp" // IWYU pragma: export

/**
 * @brief The main namespace for the iosched library.
 *
 * This namespace contains the core components for I/O scheduling.
 */
namespace io {
/**
 * @brief A customization point object for binding a socket to an address.
 *
 * This CPO can be used to bind a socket to a specific address. The actual
 * implementation is found by `tag_invoke`.
 *
 * Example:
 * @code
 * io::socket_handle sock{AF_INET, SOCK_STREAM, IPPROTO_TCP};
 * sockaddr_in addr{}
 * addr.sin_family = AF_INET;
 * addr.sin_addr.s_addr = INADDR_ANY;
 * addr.sin_port = 0;
 * // ...
 * ::io::bind(sock, &addr, sizeof(addr));
 * @endcode
 */
inline constexpr detail::bind_fn bind{};

/**
 * @brief A customization point object for setting a socket to listen.
 *
 * This CPO can be used to set a socket to listen. The actual
 * implementation is found by `tag_invoke`.
 *
 * Example:
 * @code
 * io::socket_handle sock{AF_INET, SOCK_STREAM, IPPROTO_TCP};
 * // ...
 * int backlog = 16;
 * ::io::listen(sock, backlog);
 * @endcode
 */
inline constexpr detail::listen_fn listen{};
} // namespace io
#endif // IO_TAGS_HPP
