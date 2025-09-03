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
#ifndef IO_SOCKET_DIALOG_HPP
#define IO_SOCKET_DIALOG_HPP
// #include <boost/predef.h>

// #if BOOST_OS_WINDOWS
// #include "platforms/windows/socket.hpp"
// #else
// #include "platforms/posix/socket.hpp"
// #endif

#include "socket_handle.hpp"
#include <io/execution/executor.hpp>

#include <memory>

namespace io::socket {

/**
 * @brief A dialog between a socket and an executor.
 * @tparam Mux The multiplexer type.
 */
template <::io::execution::Multiplexer Mux> struct socket_dialog {
  /**
   * @brief A weak pointer to the executor.
   */
  using executor_ptr = std::weak_ptr<::io::execution::executor<Mux>>;
  /**
   * @brief A weak pointer to the socket handle.
   */
  using socket_ptr = std::weak_ptr<socket_handle>;

  /**
   * @brief The executor for the dialog.
   */
  executor_ptr executor;
  /**
   * @brief The socket for the dialog.
   */
  socket_ptr socket;
};

} // namespace io::socket

#endif // IO_SOCKET_DIALOG_HPP
