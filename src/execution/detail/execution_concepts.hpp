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
#ifndef IO_EXECUTION_CONCEPTS_HPP
#define IO_EXECUTION_CONCEPTS_HPP

#include <type_traits>

namespace io::execution::detail {

template <typename Fn, typename E>
concept Completion = std::is_invocable_v<Fn, E *>;

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
  mux.submit(typename T::event_type{}, [](typename T::event_type *) {});
  mux.run_once_for(typename T::interval_type{});
};

} // namespace io::execution::detail
#endif // IO_EXECUTION_CONCEPTS_HPP
