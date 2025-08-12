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
#include "socket_buffer.hpp"

namespace iosched::buffers {

socket_buffer_base::socket_buffer_base(native_socket_type sock,
                                       buffer_type rbuf, buffer_type wbuf)
    : socket{sock}, read_buffer{std::move(rbuf)},
      write_buffer{std::move(wbuf)} {}

socket_buffer_base::socket_buffer_base(const socket_buffer_base &other) {
  std::lock_guard lock{other.mtx};

  // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
  socket = other.socket;
  read_buffer = other.read_buffer;
  write_buffer = other.write_buffer;
  // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
}

socket_buffer_base::socket_buffer_base(socket_buffer_base &&other) noexcept
    : socket_buffer_base() {
  swap(*this, other);
}

auto socket_buffer_base::operator=(const socket_buffer_base &other)
    -> socket_buffer_base & {
  auto temp = socket_buffer_base{other};
  swap(*this, temp);
  return *this;
}

auto socket_buffer_base::operator=(socket_buffer_base &&other) noexcept
    -> socket_buffer_base & {
  swap(*this, other);
  return *this;
}

auto swap(socket_buffer_base &lhs, socket_buffer_base &rhs) noexcept -> void {
  std::scoped_lock lock{lhs.mtx, rhs.mtx};

  using std::swap;
  swap(lhs.socket, rhs.socket);
  swap(lhs.read_buffer, rhs.read_buffer);
  swap(lhs.write_buffer, rhs.write_buffer);
}

auto socket_read_buffer::read() -> std::shared_ptr<socket_message> {
  std::lock_guard lock{mtx};
  if (read_buffer.empty())
    return {};
  auto msg = read_buffer.front();
  read_buffer.pop();
  return msg;
}

auto socket_write_buffer::write(socket_message msg) -> void {
  std::lock_guard lock{mtx};
  write_buffer.push(std::make_shared<socket_message>(std::move(msg)));
}

} // namespace iosched::buffers
