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
#ifndef IO_ACCEPT_HPP
#define IO_ACCEPT_HPP

/**
 * @file accept.hpp
 * @brief Defines the `accept` customization point object for the `io` library.
 * @details This file contains the necessary components to define the
 * `io::accept` customization point object (CPO). This CPO allows for accepting
 * new connections on socket-like objects in a generic way.
 */

#include <utility>

/**
 * @brief The main namespace for the io library.
 */
namespace io {
/**
 * @brief A tag type for the `io::accept` customization point object (CPO).
 *
 * @details This type is used as a tag to dispatch to the correct `tag_invoke`
 * overload for the `accept` CPO. It is not meant to be used directly by
 * end-users of the library. Its primary purpose is to allow library developers
 * and users to provide custom implementations of the `accept` operation for
 * their own types.
 *
 * @see io::accept
 * @see tag_invoke
 */
struct accept_t {};

/**
 * @brief Implementation details for the `io` library.
 * @details This namespace contains types and functions that are not part of the
 * public API. They are subject to change without notice.
 */
namespace detail {
/**
 * @brief The function object that implements the `accept` customization point.
 *
 * @details This struct acts as a customization point for accepting new
 * connections. It does not perform the accept operation itself, but rather
 * dispatches to a user-provided implementation via `tag_invoke`.
 *
 * To customize `accept` for a type, provide an overload of `tag_invoke` that
 * takes `io::accept_t` as its first argument.
 *
 * @see io::accept
 * @see io::accept_t
 * @see tag_invoke
 */
struct accept_fn {
  /**
   * @brief Invokes the `accept` customization point.
   *
   * @details This function call is dispatched to an overload of `tag_invoke`.
   * The first argument to `tag_invoke` will be `::io::accept_t{}`, followed by
   * the arguments passed to this function.
   *
   * @tparam Args The types of the arguments to forward to the `accept`
   * implementation.
   * @param args The arguments to forward to the `accept` implementation.
   * @return The value returned by the selected `tag_invoke` overload.
   */
  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(tag_invoke(::io::accept_t{}, std::forward<Args>(args)...)) {
    return tag_invoke(::io::accept_t{}, std::forward<Args>(args)...);
  }
};
} // namespace detail
} // namespace io
#endif // IO_ACCEPT_HPP
