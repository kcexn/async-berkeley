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

socket_handle::socket_handle(socket_handle &&other) noexcept : socket_handle() {
  swap(*this, other);
}

auto socket_handle::operator=(socket_handle &&other) noexcept
    -> socket_handle & {
  if (this != &other)
    swap(*this, other);
  return *this;
}

socket_handle::socket_handle(int domain, int type, int protocol)
    : socket_{::socket(domain, type, protocol)} {
  if (socket_ == INVALID_SOCKET)
    throw std::system_error(errno, std::generic_category(),
                            IOSCHED_ERROR_MESSAGE("Failed to create socket."));
}

socket_handle::~socket_handle() { close(); }

socket_handle::operator native_socket_type() const noexcept {
  std::lock_guard lock{mtx_};
  return socket_;
}

socket_handle::operator bool() const noexcept {
  std::lock_guard lock{mtx_};
  return socket_ != INVALID_SOCKET;
}

auto socket_handle::operator<=>(const socket_handle &other) const noexcept
    -> std::strong_ordering {
  std::scoped_lock lock(mtx_, other.mtx_);
  return socket_ <=> other.socket_;
}

auto socket_handle::operator!=(const socket_handle &other) const noexcept
    -> bool {
  return (*this <=> other) != 0;
}

auto socket_handle::close() noexcept -> void {
  std::lock_guard lock{mtx_};
  if (socket_ != INVALID_SOCKET) {
    ::iosched::socket::close(socket_);
    socket_ = INVALID_SOCKET;
  }
}

auto swap(socket_handle &lhs, socket_handle &rhs) noexcept -> void {
  std::scoped_lock lock(lhs.mtx_, rhs.mtx_);

  using std::swap;
  swap(lhs.socket_, rhs.socket_);
}

} // namespace iosched::socket
