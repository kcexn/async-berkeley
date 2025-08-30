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
 * @file executor.hpp
 * @brief This file defines a generic executor for the I/O library.
 */
#pragma once
#ifndef IO_EXECUTOR_HPP
#define IO_EXECUTOR_HPP
#include "detail/concepts.hpp"
#include "detail/immovable.hpp"

#include <exec/async_scope.hpp>
#include <stdexec/execution.hpp>

#include <utility>

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief A sender for the executor.
 * @tparam Fn The function type.
 */
template <typename Fn>
  requires std::is_invocable_v<Fn>
struct executor_sender {
  using sender_concept = stdexec::sender_t;
  using completion_signatures =
      stdexec::completion_signatures<stdexec::set_value_t(std::size_t)>;

  /**
   * @brief An operation state for the executor.
   * @tparam Receiver The receiver type.
   */
  template <typename Receiver> struct state : public immovable {
    /**
     * @brief Starts the operation.
     */
    auto start() noexcept -> void {
      stdexec::set_value(std::move(receiver), func());
    }

    Receiver receiver{};
    Fn func{};
  };

  /**
   * @brief Connects the sender to a receiver.
   * @param receiver The receiver to connect to.
   * @return The operation state.
   */
  template <typename Receiver>
  auto connect(Receiver receiver) -> state<Receiver> {
    return {.receiver = std::move(receiver), .func = std::move(func)};
  }

  Fn func{};
};

/**
 * @brief An executor that uses a multiplexer to wait for events.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> class executor : public Mux {
  using size_type = Mux::size_type;
  using interval_type = Mux::interval_type;

public:
  /**
   * @brief Sets a completion handler for an event.
   * @param event The event to wait for.
   * @param exec The completion handler.
   * @return A future that will complete when the event occurs.
   */
  template <Completion Fn>
  constexpr auto set(typename Mux::event_type event,
                     Fn &&exec) -> decltype(auto) {
    return scope_.spawn_future(
        Mux::set(std::move(event), std::forward<Fn>(exec)));
  }

  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return The number of events that occurred.
   */
  constexpr auto wait_for(int interval = -1) -> decltype(auto) {
    return Mux::wait_for(interval_type{interval});
  }

  /**
   * @brief Waits for events to occur.
   * @return The number of events that occurred.
   */
  constexpr auto wait() -> decltype(auto) { return wait_for(); }

  /**
   * @brief Gets the async scope.
   * @return The async scope.
   */
  [[nodiscard]] auto scope() noexcept -> exec::async_scope & { return scope_; }

private:
  exec::async_scope scope_;
};

} // namespace io::execution
#endif // IO_EXECUTOR_HPP
