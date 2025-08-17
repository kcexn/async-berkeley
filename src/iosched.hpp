/* Copyright 2025 Kevin Exton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is a "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifndef IOSCHED_HPP
#define IOSCHED_HPP
/**
 * @file iosched.hpp
 * @brief Core types for the iosched library.
 */

/**
 * @brief The main namespace for the iosched library.
 *
 * This namespace contains the core components for I/O scheduling.
 */
namespace iosched {
/**
 * @brief A tag type used for the `bind` customization point object (CPO).
 *
 * This type is used to dispatch to the correct `tag_invoke` overload for the
 * `bind` CPO. It is not meant to be used directly by users.
 *
 * @see iosched::bind
 * @see tag_invoke
 */
struct bind_t {};
} // namespace iosched
#endif // IOSCHED_HPP