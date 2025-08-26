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
#include "detail/immovable.hpp"

#include <exec/async_scope.hpp>
#include <stdexec/execution.hpp>

#include <utility>

namespace io::execution {

template <typename Receiver, typename Fn>
  requires std::is_invocable_v<Fn>
struct executor_op : public detail::immovable {

  auto start() noexcept -> void {
    stdexec::set_value(std::move(receiver), func());
  }

  Receiver receiver{};
  Fn func{};
};

template <typename Fn>
  requires std::is_invocable_v<Fn>
struct executor_sender {
  using sender_concept = stdexec::sender_t;
  using completion_signatures =
      stdexec::completion_signatures<stdexec::set_value_t(std::size_t)>;

  template <typename Receiver>
  auto connect(Receiver receiver) -> executor_op<Receiver, Fn> {
    return {.receiver = std::move(receiver), .func = std::move(func)};
  }

  Fn func{};
};

template <detail::Multiplexer Mux> class executor : public Mux {
  using size_type = Mux::size_type;
  using interval_type = Mux::interval_type;

public:
  template <detail::Completion<typename Mux::event_type> Fn>
  constexpr auto submit(typename Mux::event_type event,
                        Fn &&exec) -> decltype(auto) {
    return stdexec::ensure_started(
        scope_.nest(Mux::submit(std::move(event), std::forward<Fn>(exec))));
  }

  constexpr auto run_once_for(int interval = -1) -> decltype(auto) {
    return scope_.nest(executor_sender{[&, interval]() {
      return Mux::run_once_for(interval_type{interval});
    }});
  }

  constexpr auto run_once() -> decltype(auto) { return run_once_for(); }

  [[nodiscard]] auto scope() noexcept -> exec::async_scope & { return scope_; }

private:
  exec::async_scope scope_;
};

} // namespace io::execution
#endif // IO_EXECUTOR_HPP
