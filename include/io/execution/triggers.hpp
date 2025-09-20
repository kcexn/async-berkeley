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
 * @file triggers.hpp
 * @brief This file defines a high-level interface for an executor.
 */
#pragma once
#ifndef IO_TRIGGERS_HPP
#define IO_TRIGGERS_HPP
#include "detail/executor.hpp"
#include "io/detail/concepts.hpp"
#include "io/socket/socket_dialog.hpp"
#include "io/socket/socket_handle.hpp"

#include <memory>
/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {
/**
 * @brief A class that provides a high-level interface for an executor.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> class basic_triggers {
  /**
   * @internal
   * @brief The socket handle type.
   */
  using socket_handle = ::io::socket::socket_handle;
  /**
   * @internal
   * @brief The socket dialog type.
   */
  using socket_dialog = ::io::socket::socket_dialog<Mux>;

public:
  /** @brief The executor type. */
  using executor_type = executor<Mux>;

  /** @brief Default constructor. */
  basic_triggers() = default;

  /** @brief Deleted copy constructor. */
  basic_triggers(const basic_triggers &) = delete;

  /** @brief Default move constructor. */
  basic_triggers(basic_triggers &&) = default;

  /** @brief Construct with an allocator. */
  template <AllocatorLike Allocator>
  explicit basic_triggers(const Allocator &alloc = Allocator()) noexcept(
      noexcept(Allocator()))
      : executor_{std::make_shared<executor_type>(alloc)}
  {}

  /** @brief Deleted copy assignment operator. */
  auto operator=(const basic_triggers &) -> basic_triggers & = delete;

  /** @brief Default move assignment operator. */
  auto operator=(basic_triggers &&) -> basic_triggers & = default;

  /**
   * @brief Pushes a socket handle to the collection.
   * @param handle The socket handle to push.
   * @return A weak pointer to the pushed socket handle.
   */
  template <SocketLike Socket> auto push(Socket &&handle) -> socket_dialog
  {
    return {executor_, executor_type::push(std::forward<Socket>(handle))};
  }

  /**
   * @brief Emplaces a socket handle in the collection.
   * @param ...args The arguments to forward to the socket handle constructor.
   * @return A shared pointer to the emplaced socket handle.
   */
  template <typename... Args> auto emplace(Args &&...args) -> socket_dialog
  {
    return {executor_, executor_type::emplace(std::forward<Args>(args)...)};
  }

  /**
   * @brief Sets a completion handler for an event.
   * @param socket A shared pointer to guarantee the socket lifetime.
   * @param event The event to wait for.
   * @param exec The completion handler.
   * @return A sender that will complete when the event occurs.
   */
  template <Completion Fn>
  auto set(std::shared_ptr<socket_handle> socket, execution_trigger event,
           Fn &&exec) -> decltype(auto)
  {
    return executor_->set(std::move(socket), event, std::forward<Fn>(exec));
  }

  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return A sender that will complete when events occur.
   */
  constexpr auto wait_for(int interval = -1) -> decltype(auto)
  {
    return executor_->wait_for(interval);
  }

  /**
   * @brief Waits for events to occur.
   * @return A sender that will complete when events occur.
   */
  constexpr auto wait() -> decltype(auto) { return executor_->wait(); }

  /**
   * @brief Gets the executor.
   * @return A weak pointer to the executor.
   */
  [[nodiscard]] auto
  get_executor() const noexcept -> std::weak_ptr<executor_type>
  {
    return executor_;
  }

  /** @brief Default destructor. */
  ~basic_triggers() = default;

private:
  /** @brief The underlying executor. */
  std::shared_ptr<executor_type> executor_{std::make_shared<executor_type>()};
};

} // namespace io::execution
#endif // IO_TRIGGERS_HPP
