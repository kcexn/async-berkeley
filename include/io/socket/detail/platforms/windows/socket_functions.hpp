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
 * @brief This file contains the Windows-specific socket functions.
 */
#pragma once
#ifndef IO_SOCKET_FUNCTIONS_HPP
#define IO_SOCKET_FUNCTIONS_HPP
#include "socket_types.hpp"

#include <cassert>
#include <ios>
#include <utility>

#include <ws2tcpip.h>
namespace io::socket {

/** @brief POSIX `fcntl` command to set file status flags. */
static constexpr int F_SETFL = 4;
/** @brief POSIX file status flag for non-blocking I/O. */
static constexpr int O_NONBLOCK = 2048;

inline auto close(native_socket_type socket) noexcept -> int
{
  return ::closesocket(socket);
}

template <typename... Args>
inline auto fcntl(native_socket_type socket, int cmd,
                  Args &&...args) noexcept -> int
{
  static_assert(sizeof...(args) == 1,
                "fcntl for windows is only implemented for F_SETFL and a "
                "single argument O_NONBLOCK.");
  assert(cmd == F_SETFL && "fcntl for windows is only implemented for F_SETFL");

  auto arg = std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...));
  auto mode = (arg & O_NONBLOCK) ? 1 : 0;
  switch (cmd)
  {
    case F_SETFL:
      return ioctlsocket(socket, FIONBIO, &mode);
    default:
      return SOCKET_ERROR;
  }
}

inline auto sendmsg(native_socket_type socket, const socket_message_type *msg,
                    int flags) noexcept -> std::streamsize
{
  std::streamsize len = 0;
  int error = ::WSASendMsg(socket, msg, &len, nullptr, nullptr);
  return (error == 0) ? len : error;
}

inline auto recvmsg(native_socket_type socket, socket_message_type *msg,
                    int flags) noexcept -> std::streamsize
{
  std::streamsize len = 0;
  int error = ::WSARecvMsg(socket, msg, &len, nullptr, nullptr);
  return (error == 0) ? len : error;
}

} // namespace io::socket
#endif // IO_SOCKET_FUNCTIONS_HPP
