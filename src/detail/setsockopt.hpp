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
 * @file setsockopt.hpp
 * @brief This file defines the `setsockopt` customization point object for the
 * I/O library.
 */
#pragma once
#ifndef IO_SETSOCKOPT_HPP
#define IO_SETSOCKOPT_HPP
#include <utility>

namespace io {
/**
 * @brief A tag type for the `io::setsockopt` customization point object (CPO).
 *
 * This type is used as a tag to dispatch to the correct `tag_invoke` overload
 * for the `setsockopt` CPO. It is not meant to be used directly by end-users.
 */
struct setsockopt_t {};

namespace detail {
/**
 * @brief The function object that implements the `setsockopt` customization
 * point.
 *
 * This struct acts as a customization point for setting socket options. It
 * dispatches to a user-provided implementation via `tag_invoke`.
 *
 * To customize `setsockopt` for a type, provide an overload of `tag_invoke`
 * that takes `io::setsockopt_t` as its first argument.
 */
struct setsockopt_fn {
  /**
   * @brief Invokes the `setsockopt` customization point.
   *
   * This function call is dispatched to an overload of `tag_invoke`. The first
   * argument to `tag_invoke` will be `::io::setsockopt_t{}`, followed by the
   * arguments passed to this function.
   *
   * @tparam Args The types of the arguments to forward to the `setsockopt`
   * implementation.
   * @param ...args The arguments to forward to the `setsockopt`
   * implementation.
   * @return The value returned by the selected `tag_invoke` overload.
   */
  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(tag_invoke(::io::setsockopt_t{},
                             std::forward<Args>(args)...)) {
    return tag_invoke(::io::setsockopt_t{}, std::forward<Args>(args)...);
  }
};
} // namespace detail
} // namespace io
#endif // IO_SETSOCKOPT_HPP
