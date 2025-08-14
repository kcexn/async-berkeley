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
 * @brief Defines a high-level abstraction for managing a socket communication
 * session.
 *
 * This file provides the `socket_dialog` class, which extends the basic
 * `socket_handle` with state management for a complete communication
 * "dialog," including local and remote addresses, operational modes, and
 * connection state (passive/active).
 */
#pragma once
#ifndef IOSCHED_SOCKET_DIALOG_HPP
#define IOSCHED_SOCKET_DIALOG_HPP
#include "../socket/socket_address.hpp"
#include "../socket/socket_handle.hpp"

#include <bitset>

/**
 * @namespace iosched::dialog
 * @brief Provides classes for managing high-level socket communication
 * sessions.
 */
namespace iosched::dialog {
using socket_handle = ::iosched::socket::socket_handle;
using socket_address = ::iosched::socket::socket_address;

/**
 * @class socket_dialog
 * @brief Manages a socket communication session, extending `socket_handle`.
 *
 * This class encapsulates the state and behavior of a socket dialog,
 * including its local and remote endpoints, whether it is a passive
 * (listening) or active socket, and its operational mode (e.g., read/write).
 * It provides a higher-level interface for common socket operations like
 * binding.
 */
class socket_dialog : public socket_handle {

public:
  /**
   * @brief Binds the socket to a specific local address.
   *
   * This function associates the socket with a local address, which is a
   * necessary step for server sockets before they can accept connections.
   *
   * @param address The `socket_address` to bind to.
   * @throws std::system_error if the bind operation fails.
   */
  auto bind(const socket_address &address) -> void;

private:
  /**
   * @brief The local address of the socket.
   */
  socket_address local_address_;
  /**
   * @brief The remote address of the connected peer.
   */
  socket_address remote_address_;
  /**
   * @brief A flag indicating if the socket is in passive (listening) mode.
   */
  bool passive_{false};
  /**
   * @brief A bitset representing the operational mode of the socket.
   *
   * The bits can correspond to read and write permissions, as defined by
   * `iosched::socket::socket_mode`. A value of `0b11` typically means
   * both read and write are enabled.
   */
  std::bitset<2> mode_{0b11};
};

} // namespace iosched::dialog
#endif // IOSCHED_SOCKET_DIALOG_HPP
