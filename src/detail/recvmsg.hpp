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
 * @file recvmsg.hpp
 * @brief This file defines the `recvmsg` customization point object for the I/O
 * library.
 */
#pragma once
#ifndef IO_RECVMSG_HPP
#define IO_RECVMSG_HPP
#include <utility>

namespace io {
/**
 * @brief A tag type for the `io::recvmsg` customization point object (CPO).
 *
 * This type is used as a tag to dispatch to the correct `tag_invoke` overload
 * for the `recvmsg` CPO. It is not meant to be used directly by end-users.
 */
struct recvmsg_t {};

namespace detail {
/**
 * @brief The function object that implements the `recvmsg` customization point.
 *
 * This struct acts as a customization point for receiving messages. It
 * dispatches to a user-provided implementation via `tag_invoke`.
 *
 * To customize `recvmsg` for a type, provide an overload of `tag_invoke` that
 * takes `io::recvmsg_t` as its first argument.
 */
struct recvmsg_fn {
  /**
   * @brief Invokes the `recvmsg` customization point.
   *
   * This function call is dispatched to an overload of `tag_invoke`. The first
   * argument to `tag_invoke` will be `::io::recvmsg_t{}`, followed by the
   * arguments passed to this function.
   *
   * @tparam Args The types of the arguments to forward to the `recvmsg`
   * implementation.
   * @param ...args The arguments to forward to the `recvmsg` implementation.
   * @return The value returned by the selected `tag_invoke` overload.
   */
  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(tag_invoke(::io::recvmsg_t{}, std::forward<Args>(args)...)) {
    return tag_invoke(::io::recvmsg_t{}, std::forward<Args>(args)...);
  }
};
} // namespace detail
} // namespace io
#endif // IO_RECVMSG_HPP
