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
 * @brief This file contains the main header for the I/O library. It includes
 * all the necessary headers for the I/O operations and defines the main
 * namespace and customization point objects.
 */
#pragma once
#ifndef IO_HPP
#define IO_HPP
#include "detail/accept.hpp"
#include "detail/bind.hpp"
#include "detail/connect.hpp"
#include "detail/fcntl.hpp"
#include "detail/getpeername.hpp"
#include "detail/getsockname.hpp"
#include "detail/getsockopt.hpp"
#include "detail/listen.hpp"
#include "detail/recvmsg.hpp"
#include "detail/sendmsg.hpp"
#include "detail/setsockopt.hpp"
#include "detail/shutdown.hpp"

/**
 * @brief The `io` namespace contains all the functions and classes for the I/O
 * library.
 *
 * This namespace provides a set of customization point objects (CPOs) for
 * various I/O operations. These CPOs are designed to be extensible and can be
 * customized for different types of I/O objects.
 */
namespace io {
/**
 * @brief A customization point object for binding a socket to an address.
 *
 * This CPO can be used to bind a socket to a specific address. The actual
 * implementation is found by `tag_invoke`.
 *
 * @see tag_invoke
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
 * @brief A customization point object for setting a socket to listen for
 * incoming connections.
 *
 * This CPO can be used to set a socket to listen. The actual
 * implementation is found by `tag_invoke`.
 *
 * @see tag_invoke
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
 *
 * @see tag_invoke
 */
inline constexpr detail::connect_fn connect{};

/**
 * @brief A customization point object that accepts an incoming connection on a
 * listening socket.
 *
 * This CPO finds a suitable implementation for accepting a connection via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::accept_fn accept{};

/**
 * @brief A customization point object that sends a message on a socket.
 *
 * This CPO finds a suitable implementation for sending a message via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::sendmsg_fn sendmsg{};

/**
 * @brief A customization point object that receives a message from a socket.
 *
 * This CPO finds a suitable implementation for receiving a message via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::recvmsg_fn recvmsg{};

/**
 * @brief A customization point object that gets a socket option.
 *
 * This CPO finds a suitable implementation for getting a socket option via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::getsockopt_fn getsockopt{};

/**
 * @brief A customization point object that sets a socket option.
 *
 * This CPO finds a suitable implementation for setting a socket option via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::setsockopt_fn setsockopt{};

/**
 * @brief A customization point object that gets the local address of a socket.
 *
 * This CPO finds a suitable implementation for getting the local address of a
 * socket via `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::getsockname_fn getsockname{};

/**
 * @brief A customization point object that gets the peer address of a connected
 * socket.
 *
 * This CPO finds a suitable implementation for getting the peer address of a
 * socket via `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::getpeername_fn getpeername{};

/**
 * @brief A customization point object that shuts down all or part of a
 * connection on a socket.
 *
 * This CPO finds a suitable implementation for shutting down a socket via
 * `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::shutdown_fn shutdown{};

/**
 * @brief A customization point object that performs a file control operation on
 * a socket.
 *
 * This CPO finds a suitable implementation for performing a file control
 * operation on a socket via `tag_invoke`.
 *
 * @see tag_invoke
 */
inline constexpr detail::fcntl_fn fcntl{};
} // namespace io
#endif // IO_HPP
