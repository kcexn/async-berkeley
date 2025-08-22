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
#ifndef IO_CONTEXT_HPP
#define IO_CONTEXT_HPP
#include "executor.hpp"
#include <socket/socket_dialog.hpp>
#include <socket/socket_handle.hpp>

#include <map>
#include <memory>

namespace io::execution {

class context_base {

public:
  using native_socket_type = ::io::socket::native_socket_type;
  using socket_handle = ::io::socket::socket_handle;
  using handles_type =
      std::map<native_socket_type, std::shared_ptr<socket_handle>>;

  context_base() = default;
  context_base(const context_base &other);
  auto operator=(const context_base &other) -> context_base &;
  context_base(context_base &&other) noexcept;
  auto operator=(context_base &&other) noexcept -> context_base &;

  template <typename K>
  auto erase(const K &key) -> void {
    std::lock_guard lock{mtx_};

    handles_.erase(static_cast<native_socket_type>(key));
  }

  auto push(socket_handle &&handle) -> std::weak_ptr<socket_handle>;

  template <typename... Args>
  auto emplace(Args &&...args) -> std::weak_ptr<socket_handle> {
    return make_handle(std::make_shared<socket_handle>(std::forward<Args>(args)...));
  }

  virtual ~context_base() = default;

private:
  handles_type handles_;
  mutable std::mutex mtx_;

  auto make_handle(std::shared_ptr<socket_handle> ptr)
      -> std::weak_ptr<socket_handle>;

  friend auto swap(context_base &lhs, context_base &rhs) noexcept -> void;
};

template <detail::Multiplexer Mux> class basic_context : public context_base {
  using Base = context_base;
  using size_type = Mux::size_type;
  using interval_type = Mux::interval_type;
  using executor_type = executor<Mux>;
  using socket_handle = Base::socket_handle;
  using socket_dialog = ::io::socket::socket_dialog<Mux>;

public:
  static constexpr auto MUX_ERROR = Mux::MUX_ERROR;

  template <detail::Operation<typename Mux::event_type> Fn>
  auto submit(typename Mux::event_type event, Fn &&exec) -> decltype(auto) {
    return executor_->submit(event, std::forward<Fn>(exec));
  }

  auto run_for(interval_type interval = interval_type{-1}) -> size_type {
    return executor_->run_for(interval);
  }

  auto run() -> size_type { return executor_->run(); }

  auto get_executor() -> std::weak_ptr<executor_type> { return executor_; }

  auto erase(const socket_dialog &dialog) -> void {
    if(auto ptr = dialog.socket.lock())
      return Base::erase(*ptr);
  }

  auto push(socket_handle &&handle) -> socket_dialog {
    return {executor_, Base::push(std::move(handle))};
  }

  template <typename... Args> auto emplace(Args &&...args) -> socket_dialog {
    return {executor_, Base::emplace(std::forward<Args>(args)...)};
  }

private:
  std::shared_ptr<executor_type> executor_{std::make_shared<executor_type>()};
};

} // namespace io::execution
#endif // IO_CONTEXT_HPP
