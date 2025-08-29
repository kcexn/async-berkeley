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
#include "executor.hpp"
#include <io/socket/socket_dialog.hpp>
#include <io/socket/socket_handle.hpp>

#include <map>
#include <memory>

/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief Base class for triggers.
 *
 * This class manages a collection of socket handles.
 */
class triggers_base {

public:
  using native_socket_type = ::io::socket::native_socket_type;
  using socket_handle = ::io::socket::socket_handle;
  using handles_type =
      std::map<native_socket_type, std::shared_ptr<socket_handle>>;

  triggers_base() = default;
  triggers_base(const triggers_base &other);
  auto operator=(const triggers_base &other) -> triggers_base &;
  triggers_base(triggers_base &&other) noexcept;
  auto operator=(triggers_base &&other) noexcept -> triggers_base &;

  /**
   * @brief Erases a socket handle from the collection.
   * @tparam K The key type.
   * @param key The key of the socket handle to erase.
   */
  template <typename K> auto erase(const K &key) -> void {
    std::lock_guard lock{mtx_};

    handles_.erase(static_cast<native_socket_type>(key));
  }

  /**
   * @brief Pushes a socket handle to the collection.
   * @param handle The socket handle to push.
   * @return A weak pointer to the pushed socket handle.
   */
  auto push(socket_handle &&handle) -> std::weak_ptr<socket_handle>;

  /**
   * @brief Emplaces a socket handle in the collection.
   * @tparam ...Args The arguments to forward to the socket handle constructor.
   * @param ...args The arguments to forward to the socket handle constructor.
   * @return A weak pointer to the emplaced socket handle.
   */
  template <typename... Args>
  auto emplace(Args &&...args) -> std::weak_ptr<socket_handle> {
    return make_handle(
        std::make_shared<socket_handle>(std::forward<Args>(args)...));
  }

  virtual ~triggers_base() = default;

private:
  handles_type handles_;
  mutable std::mutex mtx_;

  auto make_handle(std::shared_ptr<socket_handle> ptr)
      -> std::weak_ptr<socket_handle>;

  friend auto swap(triggers_base &lhs, triggers_base &rhs) noexcept -> void;
};

/**
 * @brief A class that provides a high-level interface for an executor.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> class basic_triggers : public triggers_base {
  using Base = triggers_base;
  using size_type = Mux::size_type;
  using interval_type = Mux::interval_type;
  using executor_type = executor<Mux>;
  using socket_handle = Base::socket_handle;
  using socket_dialog = ::io::socket::socket_dialog<Mux>;

public:
  static constexpr auto MUX_ERROR = Mux::MUX_ERROR;

  /**
   * @brief Sets a completion handler for an event.
   * @tparam Fn The function type.
   * @param event The event to wait for.
   * @param exec The completion handler.
   * @return A sender that will complete when the event occurs.
   */
  template <Completion Fn>
  auto set(typename Mux::event_type event, Fn &&exec) -> decltype(auto) {
    return executor_->set(std::move(event), std::forward<Fn>(exec));
  }

  /**
   * @brief Waits for events to occur.
   * @param interval The maximum time to wait for, in milliseconds.
   * @return A sender that will complete when events occur.
   */
  constexpr auto wait_for(int interval = -1) -> decltype(auto) {
    return executor_->wait_for(interval);
  }

  /**
   * @brief Waits for events to occur.
   * @return A sender that will complete when events occur.
   */
  constexpr auto wait() -> decltype(auto) { return executor_->wait(); }

  /**
   * @brief Gets the async scope.
   * @return The async scope.
   */
  constexpr auto scope() const noexcept -> decltype(auto) {
    return executor_->scope();
  }

  /**
   * @brief Gets the executor.
   * @return A weak pointer to the executor.
   */
  auto get_executor() -> std::weak_ptr<executor_type> { return executor_; }

  /**
   * @brief Erases a socket dialog from the collection.
   * @param dialog The socket dialog to erase.
   */
  auto erase(const socket_dialog &dialog) -> void {
    if (auto ptr = dialog.socket.lock())
      return Base::erase(*ptr);
  }

  /**
   * @brief Pushes a socket handle to the collection.
   * @param handle The socket handle to push.
   * @return The created socket dialog.
   */
  auto push(socket_handle &&handle) -> socket_dialog {
    return {executor_, Base::push(std::move(handle))};
  }

  /**
   * @brief Emplaces a socket handle in the collection.
   * @tparam ...Args The arguments to forward to the socket handle constructor.
   * @param ...args The arguments to forward to the socket handle constructor.
   * @return The created socket dialog.
   */
  template <typename... Args> auto emplace(Args &&...args) -> socket_dialog {
    return {executor_, Base::emplace(std::forward<Args>(args)...)};
  }

private:
  std::shared_ptr<executor_type> executor_{std::make_shared<executor_type>()};
};

} // namespace io::execution
#endif // IO_TRIGGERS_HPP
