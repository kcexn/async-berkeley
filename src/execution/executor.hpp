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
#ifndef IO_EXECUTOR_HPP
#define IO_EXECUTOR_HPP
#include "detail/execution_concepts.hpp"

#include <utility>

namespace io::execution {

template <detail::Multiplexer Mux> class executor : public Mux {
  using size_type = Mux::size_type;
  using interval_type = Mux::interval_type;

public:
  template <detail::Operation<typename Mux::event_type> Fn>
  auto submit(typename Mux::event_type event, Fn &&exec) -> decltype(auto) {
    return Mux::submit(event, std::forward(exec));
  }

  auto run_for(interval_type interval = interval_type{-1}) -> size_type {
    return Mux::run_for(std::move(interval));
  }

  auto run() -> size_type { return run_for(); }
};

} // namespace io::execution
#endif // IO_EXECUTOR_HPP
