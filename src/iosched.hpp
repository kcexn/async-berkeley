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

#pragma once
#ifndef IOSCHED_HPP
#define IOSCHED_HPP
#include <boost/asio.hpp>
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include <winsock2.h>
#endif

namespace iosched {
using io_context = boost::asio::io_context;
#if BOOST_OS_WINDOWS
using native_socket_type = ::SOCKET;
inline static constexpr native_socket_type INVALID_SOCKET = INVALID_SOCKET;
#else
using native_socket_type = int;
inline static constexpr native_socket_type INVALID_SOCKET = -1;
#endif
} // namespace iosched
#endif // IOSCHED_HPP
