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
 * @file operations.hpp
 * @brief Defines customization point objects (CPOs) for socket operations.
 *
 * This file provides a set of customization point objects (CPOs) that allow
 * for extending socket functionalities in a generic and non-intrusive way.
 * These CPOs, such as `bind`, dispatch to user-provided implementations
 * via `tag_invoke`.
 *
 * A CPO is a function object that allows for unqualified calls to be made,
 * which can then be customized for user-defined types.
 */
#pragma once
#ifndef IOSCHED_OPERATIONS_HPP
#define IOSCHED_OPERATIONS_HPP
#include "detail/bind.hpp"

/**
 * @namespace iosched
 * @brief The main namespace for the iosched library.
 */
namespace iosched {
/**
 * @brief A customization point object for binding a socket to an address.
 *
 * This CPO can be used to bind a socket to a specific address. The actual
 * implementation is found by `tag_invoke`.
 *
 * Example:
 * @code
 * iosched::socket sock;
 * iosched::socket_address addr;
 * // ...
 * iosched::bind(sock, addr);
 * @endcode
 */
inline constexpr detail::bind_fn bind{};
} // namespace iosched

#endif // IOSCHED_OPERATIONS_HPP
