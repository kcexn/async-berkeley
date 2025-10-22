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
 * @file socket_message_impl.hpp
 * @brief Defines structures for handling socket messages.
 */
#pragma once
#include <io/detail/concepts.hpp>
#ifndef IO_SOCKET_MESSAGE_IMPL_HPP
#define IO_SOCKET_MESSAGE_IMPL_HPP
#include "io/config.h"
#include "io/socket/socket_message.hpp"

#include <algorithm>
namespace io::socket {

template <AllocatorLike Allocator>
message_buffer<Allocator>::message_buffer(const Allocator &alloc) noexcept(
    noexcept(Allocator()))
    : buffer_(alloc)
{}

template <AllocatorLike Allocator>
template <ScatterGatherLike... Bufs>
constexpr message_buffer<Allocator>::message_buffer(
    const Bufs &...bufs) noexcept
{
  (push_back(bufs), ...);
}

template <AllocatorLike Allocator>
template <ScatterGatherLike Buf>
constexpr auto message_buffer<Allocator>::push_back(const Buf &buf) -> void
{
  using element_type = std::remove_pointer_t<decltype(std::ranges::data(buf))>;
  using pointer_type = std::decay_t<element_type> *;
#if OS_WINDOWS
  push_back({std::ranges::size(buf) * sizeof(element_type),
             reinterpret_cast<char *>(
                 const_cast<pointer_type>(std::ranges::data(buf)))});
#else
  push_back({const_cast<pointer_type>(std::ranges::data(buf)),
             std::ranges::size(buf) * sizeof(element_type)});
#endif // OS_WINDOWS
}

template <AllocatorLike Allocator>
constexpr auto
message_buffer<Allocator>::push_back(native_buffer_type buf) -> void
{
  buffer_.push_back(buf);
}

template <AllocatorLike Allocator>
template <typename... Args>
constexpr auto
message_buffer<Allocator>::emplace_back(Args &&...args) -> decltype(auto)
{
  return buffer_.emplace_back(std::forward<Args>(args)...);
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::begin() noexcept -> iterator
{
  return buffer_.begin();
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::begin() const noexcept -> const_iterator
{
  return buffer_.cbegin();
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::end() noexcept -> iterator
{
  return buffer_.end();
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::end() const noexcept -> const_iterator
{
  return buffer_.cend();
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::size() const noexcept -> size_type
{
  return buffer_.size();
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr auto
message_buffer<Allocator>::empty() const noexcept -> bool
{
  std::size_t len = 0;
  for (const auto &buf : buffer_)
  {
#if OS_WINDOWS
    len += buf.len;
#else
    len += buf.iov_len;
#endif // OS_WINDOWS
  }
  return len == 0;
}

template <AllocatorLike Allocator>
[[nodiscard]] constexpr message_buffer<Allocator>::operator bool()
    const noexcept
{
  return !empty();
}

template <AllocatorLike Allocator>
auto message_buffer<Allocator>::operator+=(std::size_t len) noexcept
    -> message_buffer &
{
  auto [ret, last] = std::ranges::remove_if(buffer_, [&](auto &buf) -> bool {
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

  buffer_.erase(ret, last);
  return *this;
}

template <SocketAddress Addr, AllocatorLike Allocator>
[[nodiscard]] socket_message<Addr,
                             Allocator>::operator socket_message_type() noexcept
{
  message_header header = {
      .msg_iov = buffers, .msg_control = control, .flags = flags};
  if (address)
    header.msg_name = *address;
  return static_cast<socket_message_type>(header);
}

#if OS_WINDOWS
#else
inline message_header::operator socket_message_type() noexcept
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
#endif // IO_SOCKET_MESSAGE_IMPL_HPP
