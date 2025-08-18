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
#include "detail/accept.hpp"      // IWYU pragma: export
#include "detail/bind.hpp"        // IWYU pragma: export
#include "detail/connect.hpp"     // IWYU pragma: export
#include "detail/fcntl.hpp"       // IWYU pragma: export
#include "detail/getpeername.hpp" // IWYU pragma: export
#include "detail/getsockname.hpp" // IWYU pragma: export
#include "detail/getsockopt.hpp"  // IWYU pragma: export
#include "detail/listen.hpp"      // IWYU pragma: export
#include "detail/recvmsg.hpp"     // IWYU pragma: export
#include "detail/sendmsg.hpp"     // IWYU pragma: export
#include "detail/setsockopt.hpp"  // IWYU pragma: export
#include "detail/shutdown.hpp"    // IWYU pragma: export

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

/**
 * @brief A customization point object that connects a socket to an address.
 *
 * This CPO finds a suitable implementation for connecting a socket via
 * `tag_invoke`.
 */
inline constexpr detail::connect_fn connect{};

/**
 * @brief A customization point object that accepts an incoming connection.
 *
 * This CPO finds a suitable implementation for accepting a connection via
 * `tag_invoke`.
 */
inline constexpr detail::accept_fn accept{};

/**
 * @brief A customization point object that sends a message on a socket.
 *
 * This CPO finds a suitable implementation for sending a message via
 * `tag_invoke`.
 */
inline constexpr detail::sendmsg_fn sendmsg{};

/**
 * @brief A customization point object that receives a message from a socket.
 *
 * This CPO finds a suitable implementation for receiving a message via
 * `tag_invoke`.
 */
inline constexpr detail::recvmsg_fn recvmsg{};

/**
 * @brief A customization point object that gets a socket option.
 *
 * This CPO finds a suitable implementation for getting a socket option via
 * `tag_invoke`.
 */
inline constexpr detail::getsockopt_fn getsockopt{};

/**
 * @brief A customization point object that sets a socket option.
 *
 * This CPO finds a suitable implementation for setting a socket option via
 * `tag_invoke`.
 */
inline constexpr detail::setsockopt_fn setsockopt{};

/**
 * @brief A customization point object that gets the local address of a socket.
 *
 * This CPO finds a suitable implementation for getting the local address of a
 * socket via `tag_invoke`.
 */
inline constexpr detail::getsockname_fn getsockname{};

/**
 * @brief A customization point object that gets the peer address of a connected
 * socket.
 *
 * This CPO finds a suitable implementation for getting the peer address of a
 * socket via `tag_invoke`.
 */
inline constexpr detail::getpeername_fn getpeername{};

/**
 * @brief A customization point object that shuts down all or part of a
 * connection on a socket.
 *
 * This CPO finds a suitable implementation for shutting down a socket via
 * `tag_invoke`.
 */
inline constexpr detail::shutdown_fn shutdown{};

/**
 * @brief A customization point object that performs a file control operation on
 * a socket.
 *
 * This CPO finds a suitable implementation for performing a file control
 * operation on a socket via `tag_invoke`.
 */
inline constexpr detail::fcntl_fn fcntl{};
} // namespace io
#endif // IO_HPP
