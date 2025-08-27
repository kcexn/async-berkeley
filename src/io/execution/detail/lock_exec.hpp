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
 * @file lock_exec.hpp
 * @brief This file defines a function to execute a function with a lock.
 */
#pragma once
#ifndef IO_LOCK_EXEC_HPP
#define IO_LOCK_EXEC_HPP
#include <mutex>
#include <type_traits>

/**
 * @namespace io::execution::detail
 * @brief Contains implementation details for the execution components.
 */
namespace io::execution::detail {

/**
 * @brief Executes a function with a lock.
 *
 * This function takes a unique lock and a function, and executes the function.
 * The lock is released when the function returns.
 *
 * @tparam Fn The function type.
 * @param lock The lock to hold while executing the function.
 * @param func The function to execute.
 * @return The return value of the function.
 */
template <typename Fn>
  requires std::is_invocable_v<Fn>
auto lock_exec(std::unique_lock<std::mutex> lock, Fn func) -> decltype(auto) {
  return func();
}

} // namespace io::execution::detail
#endif // IO_LOCK_EXEC_HPP
