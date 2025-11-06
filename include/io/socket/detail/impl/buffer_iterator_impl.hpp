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
 * @file buffer_iterator_impl.hpp
 * @brief Defines a proxy iterator for socket message buffers.
 */
#pragma once
#ifndef IO_BUFFER_ITERATOR_IMPL_HPP
#define IO_BUFFER_ITERATOR_IMPL_HPP
#include "io/socket/detail/buffer_iterator.hpp"
namespace io::socket {
template <typename Iterator>
constexpr buffer_iterator<Iterator>::buffer_iterator(Iterator iter) noexcept
    : it_(iter)
{}

template <typename Iterator>
constexpr auto
buffer_iterator<Iterator>::operator*() const noexcept -> reference
{
#if OS_WINDOWS
  return std::span<std::byte>(reinterpret_cast<std::byte *>(it_->buf),
                              it_->len);
#else
  return std::span<std::byte>(static_cast<std::byte *>(it_->iov_base),
                              it_->iov_len);
#endif
}

template <typename Iterator>
constexpr auto buffer_iterator<Iterator>::operator[](
    difference_type n) const noexcept -> reference
{
  return *(*this + n);
}

template <typename Iterator>
constexpr auto
buffer_iterator<Iterator>::operator++() noexcept -> buffer_iterator &
{
  ++it_;
  return *this;
}

template <typename Iterator>
constexpr auto
buffer_iterator<Iterator>::operator++(int) noexcept -> buffer_iterator
{
  auto tmp = *this;
  ++(*this);
  return tmp;
}

template <typename Iterator>
constexpr auto
buffer_iterator<Iterator>::operator--() noexcept -> buffer_iterator &
{
  --it_;
  return *this;
}

template <typename Iterator>
constexpr auto
buffer_iterator<Iterator>::operator--(int) noexcept -> buffer_iterator
{
  auto tmp = *this;
  --(*this);
  return tmp;
}

template <typename Iterator>
constexpr auto buffer_iterator<Iterator>::operator+=(difference_type n) noexcept
    -> buffer_iterator &
{
  it_ += n;
  return *this;
}

template <typename Iterator>
constexpr auto buffer_iterator<Iterator>::operator-=(difference_type n) noexcept
    -> buffer_iterator &
{
  it_ -= n;
  return *this;
}

template <typename Iterator>
constexpr auto buffer_iterator<Iterator>::base() const noexcept -> Iterator
{
  return it_;
}

} // namespace io::socket
#endif // IO_BUFFER_ITERATOR_IMPL_HPP
