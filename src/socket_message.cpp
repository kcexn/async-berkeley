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
 * @file socket_message.cpp
 */
#include "io/socket/socket_message.hpp"
#include "io/config.h"

#include <algorithm>
namespace io::socket {

auto scatter_gather_buffers::operator+=(std::size_t len) noexcept
    -> scatter_gather_buffers &
{
  auto [ret, last] = std::ranges::remove_if(buffers_, [&](auto &buf) -> bool {
    if (!len)
      return false;

#if OS_WINDOWS
    auto count = buf.len;
#else
    auto count = buf.iov_len;
#endif // OS_WINDOWS

    buf += len;
    return (len < count) ? (len = 0) : (len -= count) >= 0;
  });

  buffers_.erase(ret, last);
  return *this;
}

#if OS_WINDOWS
#else
message_header::operator socket_message_type() noexcept
{
  return {.msg_name = msg_name.data(),
          .msg_namelen = static_cast<socklen_t>(msg_name.size()),
          .msg_iov = msg_iov.data(),
          .msg_iovlen = msg_iov.size(),
          .msg_control = msg_control.data(),
          .msg_controllen = msg_control.size(),
          .msg_flags = flags};
}
#endif // OS_WINDOWS

} // namespace io::socket
