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
 * @file customization.hpp
 * @brief This file defines generic customization points
 */
#pragma once
#ifndef IO_CUSTOMIZATION_HPP
#define IO_CUSTOMIZATION_HPP

#include <utility>

namespace io::detail {
/**
 * @brief A generic function object that implements customization points.
 */
template <typename Tag> struct cpo {
  /**
   * @brief Invokes the customization point.
   * @tparam Args The types of the arguments to forward to the implementation.
   * @param ...args The arguments to forward to the implementation.
   * @return The value returned by the selected `tag_invoke` overload.
   */
  template <typename... Args>
  auto operator()(Args &&...args) const -> decltype(auto) {
    return tag_invoke(Tag{}, std::forward<Args>(args)...);
  }
};
} // namespace io::detail

#endif // IO_CUSTOMIZATION_HPP
