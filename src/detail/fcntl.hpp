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

#pragma once
#ifndef IO_FCNTL_HPP
#define IO_FCNTL_HPP
#include <utility>

/**
 * @brief The main namespace for the io library.
 */
namespace io {
/**
 * @brief A tag type used for the `fcntl` customization point object (CPO).
 *
 * This type is used to dispatch to the correct `tag_invoke` overload for the
 * `fcntl` CPO. It is not meant to be used directly by users.
 *
 * @see io::fcntl
 * @see tag_invoke
 */
struct fcntl_t {};

/**
 * @brief Implementation details for the `io` library.
 * @details This namespace contains types and functions that are not part of the
 * public API. They are subject to change without notice.
 */
namespace detail {
/**
 * @brief A function object that provides the `fcntl` customization point.
 * @details This struct acts as a customization point for file control
 * operations. It doesn't perform the fcntl itself, but dispatches to a
 * user-provided implementation via `tag_invoke`. To customize `fcntl` for a
 * type, provide an overload of `tag_invoke` with `io::fcntl_t` as the first
 * argument.
 */
struct fcntl_fn {
  /**
   * @brief Calls the `fcntl` customization point.
   *
   * This function call is dispatched to an overload of `tag_invoke`. The first
   * argument to `tag_invoke` is `::io::fcntl_t{}`, followed by the
   * arguments passed to this function.
   *
   * @tparam Args The types of the arguments.
   * @param args The arguments to pass to the `fcntl` implementation.
   * @return The value returned by the `tag_invoke` overload.
   */
  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(tag_invoke(::io::fcntl_t{}, std::forward<Args>(args)...)) {
    return tag_invoke(::io::fcntl_t{}, std::forward<Args>(args)...);
  }
};
} // namespace detail
} // namespace io
#endif // IO_FCNTL_HPP
