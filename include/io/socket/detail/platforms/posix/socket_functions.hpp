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
 * @file socket_functions.hpp
 * @brief This file contains the POSIX-specific socket functions.
 */
#pragma once
#ifndef IO_SOCKET_FUNCTIONS_HPP
#define IO_SOCKET_FUNCTIONS_HPP
#include "socket_types.hpp"

#include <ios>
#include <utility>

#include <fcntl.h>
#include <unistd.h>
namespace io::socket {

inline auto close(native_socket_type socket) noexcept -> int
{
  return ::close(socket);
}

template <typename... Args>
inline auto fcntl(native_socket_type socket, int cmd,
                  Args &&...args) noexcept -> int
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  return ::fcntl(socket, cmd, std::forward<Args>(args)...);
}

inline auto sendmsg(native_socket_type socket, const socket_message_type *msg,
                    int flags) noexcept -> std::streamsize
{
  return ::sendmsg(socket, msg, flags);
}

inline auto recvmsg(native_socket_type socket, socket_message_type *msg,
                    int flags) noexcept -> std::streamsize
{
  return ::recvmsg(socket, msg, flags);
}

} // namespace io::socket
#endif // IO_SOCKET_POSIX_HPP
