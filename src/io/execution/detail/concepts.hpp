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
 * @file concepts.hpp
 * @brief This file defines concepts for the execution components.
 */
#pragma once
#ifndef IO_CONCEPTS_HPP
#define IO_CONCEPTS_HPP
#include <type_traits>

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief Concept for a multiplexer tag.
 *
 * A multiplexer tag provides the basic types and functions required by a
 * multiplexer.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept MuxTag = requires(T tag) {
  typename T::event_type;    ///< The multiplexed event type.
  typename T::interval_type; ///< The type used to specify timeouts.
  typename T::size_type;     ///< A size type.
  /**
   * @brief Returns the key for a multiplexed event.
   * @param event The multiplexed event of event_type.
   * @return The key for the event.
   */
  T::key(typename T::event_type{});
};

/**
 * @brief Concept for a completion handler.
 * @tparam Fn The function type.
 */
template <typename Fn>
concept Completion = std::is_invocable_v<Fn>;

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
  mux.set(typename T::event_type{}, []() {});
  mux.wait_for(typename T::interval_type{});
};

} // namespace io::execution
#endif // IO_CONCEPTS_HPP
