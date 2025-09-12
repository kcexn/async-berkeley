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
#include "io/macros.h"
#if OS_WINDOWS
#include "io/socket/platforms/windows/socket.hpp"
#else
#include "io/socket/platforms/posix/socket.hpp"
#include "platforms/posix/concepts_posix.hpp" // IWYU pragma: export
#endif

#include <optional>
#include <type_traits>

/**
 * @namespace io
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io {

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
};

/**
 * @brief Concept for a completion handler.
 * @tparam Fn The function type.
 */
template <typename Fn>
concept Completion = requires(Fn &&func) {
  requires std::is_invocable_v<Fn>;
  typename std::invoke_result_t<Fn>::value_type;
  requires std::is_same_v<
      std::invoke_result_t<Fn>,
      std::optional<typename std::invoke_result_t<Fn>::value_type>>;
};

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
  requires MuxTag<typename T::multiplexer_type>;
  mux.wait_for(typename T::interval_type{});
};

/**
 * @brief Concept for types that behave like a socket.
 * @tparam Socket The type to check.
 */
template <typename Socket>
concept SocketLike = requires {
  Socket{socket::native_socket_type{}};
  std::is_convertible_v<Socket, socket::native_socket_type>;
};

/**
 * @brief Concept for types that behave like a socket message.
 * @tparam Message The type to check.
 */
template <typename Message>
concept MessageLike =
    requires { std::is_convertible_v<Message, socket::socket_message_type>; };

} // namespace io
#endif // IO_CONCEPTS_HPP
