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
#include "detail/customization.hpp"
#include "socket/socket_ops.hpp"

/**
 * @brief The `io` namespace contains all the functions and classes for the I/O
 * library.
 *
 * This namespace provides a set of customization point objects (CPOs) for
 * various I/O operations. These CPOs are designed to be extensible and can be
 * customized for different types of I/O objects.
 */
namespace io {
/// @brief A customization point object for binding a socket to an address.
inline constexpr detail::cpo<socket::bind_t> bind{};
/// @brief A customization point object for setting a socket to listen for
inline constexpr detail::cpo<socket::listen_t> listen{};
/// @brief A customization point object that connects a socket to an address.
inline constexpr detail::cpo<socket::connect_t> connect{};
/// @brief A customization point object that accepts an incoming connection on a
/// listening socket.
inline constexpr detail::cpo<socket::accept_t> accept{};
/// @brief A customization point object that sends a message on a socket.
inline constexpr detail::cpo<socket::sendmsg_t> sendmsg{};
/// @brief A customization point object that receives a message from a socket.
inline constexpr detail::cpo<socket::recvmsg_t> recvmsg{};
/// @brief A customization point object that gets a socket option.
inline constexpr detail::cpo<socket::getsockopt_t> getsockopt{};
/// @brief A customization point object that sets a socket option.
inline constexpr detail::cpo<socket::setsockopt_t> setsockopt{};
/// @brief A customization point object that gets the local address of a socket.
inline constexpr detail::cpo<socket::getsockname_t> getsockname{};
/// @brief A customization point object that gets the peer address of a
/// connected socket.
inline constexpr detail::cpo<socket::getpeername_t> getpeername{};
/// @brief A customization point object that shuts down all or part of a
/// connection on a socket.
inline constexpr detail::cpo<socket::shutdown_t> shutdown{};
/// @brief A customization point object that performs a file control operation
/// on a socket.
inline constexpr detail::cpo<socket::fcntl_t> fcntl{};
} // namespace io
#endif // IO_HPP
