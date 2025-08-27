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
 * @file execution_concepts.hpp
 * @brief This file defines concepts for the execution components.
 */
#pragma once
#ifndef IO_EXECUTION_CONCEPTS_HPP
#define IO_EXECUTION_CONCEPTS_HPP
#include <type_traits>

/**
 * @namespace io::execution::detail
 * @brief Contains implementation details for the execution components.
 */
namespace io::execution::detail {

/**
 * @brief Concept for a completion handler.
 * @tparam Fn The function type.
 * @tparam E The event type.
 */
template <typename Fn, typename E>
concept Completion = std::is_invocable_v<Fn, E>;

/**
 * @brief Concept for a multiplexer.
 *
 * A multiplexer is responsible for waiting for events and dispatching them to
 * completion handlers.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept Multiplexer = requires(T mux) {
  requires !std::is_copy_constructible_v<T>;
  requires !std::is_copy_assignable_v<T>;
  requires !std::is_move_constructible_v<T>;
  requires !std::is_move_assignable_v<T>;
  typename T::event_type;
  typename T::size_type;
  typename T::interval_type;
  T::MUX_ERROR;
  mux.set(typename T::event_type{}, [](typename T::event_type) {});
  mux.wait_for(typename T::interval_type{});
};

} // namespace io::execution::detail
#endif // IO_EXECUTION_CONCEPTS_HPP
