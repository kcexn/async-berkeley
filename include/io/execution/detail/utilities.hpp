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
 * @file utilities.hpp
 * @brief This file provides utility functions for the execution components.
 */
#pragma once
#ifndef IO_UTILITIES_HPP
#define IO_UTILITIES_HPP
#include <mutex>
#include <type_traits>

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief Executes a function with a lock.
 * @tparam Fn The function type.
 * @param lock The lock to use.
 * @param func The function to execute.
 * @return The result of the function.
 */
template <typename Fn>
  requires std::is_invocable_v<Fn>
auto with_lock(std::unique_lock<std::mutex> lock, Fn &&func) -> decltype(auto)
{
  return std::forward<Fn>(func)();
}

} // namespace io::execution
#endif // IO_UTILITIES_HPP
