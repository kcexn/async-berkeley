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
 * @file socket_dialog.hpp
 * @brief Defines the `socket_dialog` struct.
 */
#pragma once
#ifndef IO_SOCKET_DIALOG_HPP
#define IO_SOCKET_DIALOG_HPP
#include "io/detail/concepts.hpp"

#include <memory>

// Forward declarations
namespace io::execution {
template <Multiplexer Mux> class executor;
} // namespace io::execution

namespace io::socket {
class socket_handle;
} // namespace io::socket

/**
 * @namespace io::socket
 * @brief The `io::socket` namespace provides a cross-platform abstraction for
 * socket-level I/O operations.
 */
namespace io::socket {
/**
 * @brief A dialog that facilitates asynchronous operations on the
 * socket by the executor.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> struct socket_dialog {
  using executor_type = ::io::execution::executor<Mux>;

  std::weak_ptr<executor_type> executor;
  std::shared_ptr<socket_handle> socket;
};

} // namespace io::socket

#endif // IO_SOCKET_DIALOG_HPP
