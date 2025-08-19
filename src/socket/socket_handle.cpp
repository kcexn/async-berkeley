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
#include "../error.hpp"

#include <system_error>

// generic helper functions
static auto throw_system_error(const char *message) -> void {
  throw std::system_error(errno, std::generic_category(), message);
}

namespace io::socket {

static auto is_valid_socket(native_socket_type handle) -> bool {
  if (handle == INVALID_SOCKET)
    return false;

  int type = 0;
  socklen_type len = sizeof(type);

  return ::getsockopt(handle, SOL_SOCKET, SO_TYPE, &type, &len) == 0;
}

} // namespace io::socket

// socket_handle implementation
namespace io::socket {

socket_handle::socket_handle(socket_handle &&other) noexcept : socket_handle() {
  swap(*this, other);
}

auto socket_handle::operator=(socket_handle &&other) noexcept
    -> socket_handle & {
  swap(*this, other);
  return *this;
}

socket_handle::socket_handle(native_socket_type handle) : socket_{handle} {
  if (!is_valid_socket(handle))
    throw_system_error(IO_ERROR_MESSAGE("Invalid socket handle."));
}

socket_handle::socket_handle(int domain, int type, int protocol)
    : socket_{::socket(domain, type, protocol)} {
  if (socket_ == INVALID_SOCKET)
    throw_system_error(IO_ERROR_MESSAGE("Failed to create socket."));
}

socket_handle::operator native_socket_type() const noexcept {
  return socket_.load(std::memory_order_relaxed);
}

auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void {
  if (&lhs == &rhs)
    return;

  std::scoped_lock lock(lhs.mtx_, rhs.mtx_);

  using std::swap;
  auto temp = lhs.socket_.exchange(rhs.socket_);
  rhs.socket_.store(temp);
}

socket_handle::operator bool() const noexcept {
  return socket_ != INVALID_SOCKET;
}

auto socket_handle::operator<=>(const socket_handle &other) const noexcept
    -> std::strong_ordering {
  return socket_ <=> other.socket_;
}

auto socket_handle::operator==(const socket_handle &other) const noexcept
    -> bool {
  return (*this <=> other) == 0;
}

auto socket_handle::operator<=>(native_socket_type other) const noexcept
    -> std::strong_ordering {
  return socket_ <=> other;
}

auto socket_handle::operator==(native_socket_type other) const noexcept
    -> bool {
  return (*this <=> other) == 0;
}

socket_handle::~socket_handle() { close(); }

auto socket_handle::close() noexcept -> void {
  std::lock_guard lock{mtx_};
  if (socket_ != INVALID_SOCKET) {
    ::io::socket::close(socket_);
    socket_ = INVALID_SOCKET;
  }
}

} // namespace io::socket
