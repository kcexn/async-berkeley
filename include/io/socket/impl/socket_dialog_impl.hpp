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
#ifndef IO_SOCKET_DIALOG_IMPL_HPP
#define IO_SOCKET_DIALOG_IMPL_HPP
#include "io/error.hpp"
#include "io/socket/socket_dialog.hpp"

namespace io::socket {

template <Multiplexer Mux> socket_dialog<Mux>::operator bool() const noexcept
{
  return !executor.expired() && socket && static_cast<bool>(*socket);
}

template <Multiplexer Mux>
socket_dialog<Mux>::operator native_socket_type() const
{
  if (!socket)
    throw std::invalid_argument(IO_ERROR_MESSAGE("Invalid socket pointer."));

  return static_cast<native_socket_type>(*socket);
}

template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 const socket_dialog<Mux> &rhs) -> std::strong_ordering
{
  if (!lhs.socket || !rhs.socket)
    throw std::invalid_argument(IO_ERROR_MESSAGE("Invalid socket pointer."));

  return *lhs.socket <=> *rhs.socket;
}

template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs,
                const socket_dialog<Mux> &rhs) -> bool
{
  return (lhs <=> rhs) == 0;
}

template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 const socket_handle &rhs) -> std::strong_ordering
{
  if (!lhs.socket)
    throw std::invalid_argument(IO_ERROR_MESSAGE("Invalid socket pointer."));
  return *lhs.socket <=> rhs;
}

template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs, const socket_handle &rhs) -> bool
{
  return (lhs <=> rhs) == 0;
}

template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 native_socket_type rhs) -> std::strong_ordering
{
  if (!lhs.socket)
    throw std::invalid_argument(IO_ERROR_MESSAGE("Invalid socket pointer."));

  return *lhs.socket <=> rhs;
}

template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs, native_socket_type rhs) -> bool
{
  return (lhs <=> rhs) == 0;
}

} // namespace io::socket

#endif // IO_SOCKET_DIALOG_IMPL_HPP
