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
#include "socket_handle.hpp"
#include "../error/error_handling.hpp"
#include <system_error>

namespace iosched::socket {

socket_handle_state::socket_handle_state(const socket_handle_state &other)
    : socket_handle_state() {
  socket.store(other.socket);
}

auto socket_handle_state::operator=(const socket_handle_state &other)
    -> socket_handle_state & {
  auto temp = other;
  swap(*this, temp);
  return *this;
}

socket_handle_state::socket_handle_state(socket_handle_state &&other) noexcept
    : socket_handle_state() {
  swap(*this, other);
}

auto socket_handle_state::operator=(socket_handle_state &&other) noexcept
    -> socket_handle_state & {
  swap(*this, other);
  return *this;
}

socket_handle_state::socket_handle_state(native_socket_type handle) noexcept
    : socket{handle} {}

socket_handle_state::socket_handle_state(int domain, int type, int protocol)
    : socket{::socket(domain, type, protocol)} {
  if (socket == INVALID_SOCKET)
    throw std::system_error(errno, std::generic_category(),
                            IOSCHED_ERROR_MESSAGE("Failed to create socket."));
}

auto swap(socket_handle_state &lhs, socket_handle_state &rhs) noexcept -> void {
  auto temp = lhs.socket.exchange(rhs.socket.load());
  rhs.socket.store(temp);
}

socket_handle_state::operator bool() const noexcept {
  return socket != INVALID_SOCKET;
}

auto socket_handle_state::operator<=>(
    const socket_handle_state &other) const noexcept -> std::strong_ordering {
  return socket <=> other.socket;
}

auto socket_handle_state::operator==(
    const socket_handle_state &other) const noexcept -> bool {
  return (*this <=> other) == 0;
}

auto socket_handle_state::operator<=>(native_socket_type other) const noexcept
    -> std::strong_ordering {
  return socket <=> other;
}

auto socket_handle_state::operator==(native_socket_type other) const noexcept
    -> bool {
  return (*this <=> other) == 0;
}

socket_handle::socket_handle(socket_handle &&other) noexcept : socket_handle() {
  swap(*this, other);
}

auto socket_handle::operator=(socket_handle &&other) noexcept
    -> socket_handle & {
  if (this != &other)
    swap(*this, other);
  return *this;
}

auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void {
  std::scoped_lock lock(lhs.mtx_, rhs.mtx_);

  using Base = socket_handle::Base;
  using std::swap;
  swap(static_cast<Base &>(lhs), static_cast<Base &>(rhs));
}

auto socket_handle::operator<=>(const socket_handle &other) const noexcept
    -> std::strong_ordering {
  return static_cast<const Base &>(*this) <=> static_cast<const Base &>(other);
}

auto socket_handle::operator==(const socket_handle &other) const noexcept
    -> bool {
  return (*this <=> other) == 0;
}

socket_handle::~socket_handle() { close(); }

auto socket_handle::close() noexcept -> void {
  std::lock_guard lock{mtx_};
  if (socket != INVALID_SOCKET) {
    ::iosched::socket::close(socket);
    socket = INVALID_SOCKET;
  }
}

} // namespace iosched::socket
