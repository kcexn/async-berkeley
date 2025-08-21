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

namespace io::execution::detail {

template <typename T>
concept Event = requires(T event) { event.key(); };

template <typename F, typename E>
concept Operation = requires(F func) {
  func(E{});
  requires Event<E>;
};

template <typename T>
concept Multiplexer = requires(T mux) {
  typename T::event_type;
  typename T::size_type;
  typename T::interval_type;
  requires Event<typename T::event_type>;
  mux.submit(typename T::event_type{}, [](typename T::event_type) {});
  mux.wait(typename T::interval_type{});
};

} // namespace io::execution::detail
#endif // IO_EXECUTION_CONCEPTS_HPP
