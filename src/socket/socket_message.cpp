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

#include "socket_message.hpp"

namespace io::socket {

socket_message::socket_message() : data_{std::make_unique<message_data>()} {}

socket_message::socket_message(socket_message &&other) noexcept
    : socket_message() {
  swap(*this, other);
}

auto socket_message::operator=(socket_message &&other) noexcept
    -> socket_message & {
  swap(*this, other);
  return *this;
}

auto swap(socket_message &lhs, socket_message &rhs) noexcept -> void {
  if (&lhs == &rhs)
    return;

  std::scoped_lock lock(lhs.mtx_, rhs.mtx_);

  using std::swap;
  swap(lhs.data_, rhs.data_);
}

auto socket_message::get() const -> message_data {
  std::lock_guard lock{mtx_};

  return *data_;
}

auto socket_message::operator=(message_data data) -> socket_message & {
  std::lock_guard lock{mtx_};
  data_ = std::make_unique<message_data>(std::move(data));
  return *this;
}

auto socket_message::address() const -> socket_address {
  std::lock_guard lock{mtx_};
  return data_->address;
}

auto socket_message::set_address(socket_address address) -> socket_message & {
  std::lock_guard lock{mtx_};
  data_->address = address;
  return *this;
}

auto socket_message::exchange_address(socket_address address)
    -> socket_address {
  std::lock_guard lock{mtx_};
  using std::swap;
  swap(data_->address, address);
  return address;
}

auto socket_message::buffers() const -> scatter_gather_buffer {
  std::lock_guard lock{mtx_};
  return data_->buffers;
}

auto socket_message::set_buffers(scatter_gather_buffer buffers)
    -> socket_message & {
  std::lock_guard lock{mtx_};
  data_->buffers = std::move(buffers);
  return *this;
}

auto socket_message::exchange_buffers(scatter_gather_buffer buffers)
    -> scatter_gather_buffer {
  std::lock_guard lock{mtx_};
  using std::swap;
  swap(data_->buffers, buffers);
  return buffers;
}

auto socket_message::push_back(socket_buffer_type buffer) -> socket_message & {
  std::lock_guard lock{mtx_};
  data_->buffers.push_back(buffer);
  return *this;
}

auto socket_message::control() const -> ancillary_buffer {
  std::lock_guard lock{mtx_};
  return data_->control;
}

auto socket_message::set_control(ancillary_buffer control) -> socket_message & {
  std::lock_guard lock{mtx_};
  data_->control = std::move(control);
  return *this;
}

auto socket_message::exchange_control(ancillary_buffer control)
    -> ancillary_buffer {
  std::lock_guard lock{mtx_};
  using std::swap;
  swap(data_->control, control);
  return control;
}

auto socket_message::flags() const -> int {
  std::lock_guard lock{mtx_};
  return data_->flags;
}

auto socket_message::set_flags(int flags) -> socket_message & {
  std::lock_guard lock{mtx_};
  data_->flags = flags;
  return *this;
}

auto socket_message::exchange_flags(int flags) -> int {
  std::lock_guard lock{mtx_};
  using std::swap;
  swap(data_->flags, flags);
  return flags;
}

} // namespace io::socket
