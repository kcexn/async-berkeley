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
#include "triggers.hpp"

namespace io::execution {

auto triggers_base::make_handle(std::shared_ptr<socket_handle> ptr)
    -> std::weak_ptr<socket_handle> {
  std::lock_guard lock{mtx_};

  auto native_handle = static_cast<native_socket_type>(*ptr);
  auto [handles_it, emplaced] =
      handles_.try_emplace(native_handle, std::move(ptr));
  if (!emplaced)
    handles_it->second = std::move(ptr);
  return handles_it->second;
}

auto swap(triggers_base &lhs, triggers_base &rhs) noexcept -> void {
  using std::swap;
  if (&lhs == &rhs)
    return;

  std::scoped_lock lock{lhs.mtx_, rhs.mtx_};
  swap(lhs.handles_, rhs.handles_);
}

triggers_base::triggers_base(const triggers_base &other) : triggers_base() {
  std::lock_guard lock{other.mtx_};

  handles_ = other.handles_;
}

auto triggers_base::operator=(const triggers_base &other) -> triggers_base & {
  using std::swap;

  auto tmp = other;
  swap(*this, tmp);
  return *this;
}

triggers_base::triggers_base(triggers_base &&other) noexcept : triggers_base() {
  swap(*this, other);
}

auto triggers_base::operator=(triggers_base &&other) noexcept
    -> triggers_base & {
  using std::swap;
  swap(*this, other);
  return *this;
}

auto triggers_base::push(socket_handle &&handle)
    -> std::weak_ptr<socket_handle> {
  return make_handle(std::make_shared<socket_handle>(std::move(handle)));
}

} // namespace io::execution
