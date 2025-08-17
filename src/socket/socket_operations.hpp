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
#ifndef IOSCHED_SOCKET_OPERATIONS_HPP
#define IOSCHED_SOCKET_OPERATIONS_HPP

#include <utility>

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 */
namespace iosched::socket {

namespace detail {
struct bind_fn {
  template <typename... Args> auto operator()(Args &&...args) {
    return tag_invoke();

    bind(std::forward<Args>(args)...);
  }
};
} // namespace detail

inline constexpr detail::bind_fn bind{};

} // namespace iosched::socket

#endif // IOSCHED_SOCKET_OPERATIONS_HPP
