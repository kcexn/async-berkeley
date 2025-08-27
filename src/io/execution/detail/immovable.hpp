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
 * @file immovable.hpp
 * @brief This file defines a struct to make a class immovable.
 */
#pragma once
#ifndef IO_IMMOVABLE_HPP
#define IO_IMMOVABLE_HPP

/**
 * @namespace io::execution::detail
 * @brief Contains implementation details for the execution components.
 */
namespace io::execution::detail {
/**
 * @brief A base struct to make a class immovable.
 *
 * This struct deletes the move constructor and move assignment operator, making
 * any class that inherits from it immovable.
 */
struct immovable {
  immovable() = default;
  immovable(const immovable &) = default;
  auto operator=(const immovable &) -> immovable & = default;
  immovable(immovable &&) = delete;
  auto operator=(immovable &&) noexcept -> immovable & = delete;
  ~immovable() = default;
};
} // namespace io::execution::detail

#endif // IO_IMMOVABLE_HPP
