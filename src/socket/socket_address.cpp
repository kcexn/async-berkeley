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

#include "socket_address.hpp"

#include <cstring>

namespace iosched::socket {

socket_address::socket_address(const sockaddr_type *addr,
                               socklen_type size) noexcept
    : size_{size} {
  std::memcpy(&storage_, addr, size_);
}

auto socket_address::data() noexcept -> sockaddr_type * {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<sockaddr_type *>(&storage_);
}

auto socket_address::data() const noexcept -> const sockaddr_type * {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<const sockaddr_type *>(&storage_);
}

auto socket_address::size() noexcept -> socklen_type * { return &size_; }

auto socket_address::size() const noexcept -> const socklen_type * {
  return &size_;
}

auto socket_address::operator==(const socket_address &other) const noexcept
    -> bool {
  if (size_ != other.size_)
    return false;
  return std::memcmp(&storage_, &other.storage_, size_) == 0;
}
} // namespace iosched::socket
