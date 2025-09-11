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
 * @file async_operations.hpp
 * @brief Defines the available socket operations using the tag_invoke
 * customization point.
 */
#pragma once
#ifndef IO_ASYNC_OPERATIONS_HPP
#define IO_ASYNC_OPERATIONS_HPP
#include <boost/predef.h>
#if BOOST_OS_WINDOWS
#include "io/socket/platforms/windows/socket.hpp"
#else
#include "io/socket/platforms/posix/socket.hpp"
#endif
#include "io/detail/customization.hpp"
#include "io/error.hpp"
#include "io/execution/detail/execution_trigger.hpp"
#include "io/socket/socket_dialog.hpp"

#include <stdexec/execution.hpp>

#include <optional>
namespace io::socket {
namespace detail {
/**
 * @brief Gets the executor from a socket dialog.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog to get the executor from.
 * @return A shared pointer to the executor.
 * @throw std::invalid_argument if the executor is invalid.
 */
template <Multiplexer Mux>
auto get_executor(const socket_dialog<Mux> &dialog)
    -> std::shared_ptr<typename socket_dialog<Mux>::executor_type> {
  if (auto executor = dialog.executor.lock())
    return std::move(executor);

  throw std::invalid_argument(IO_ERROR_MESSAGE("Invalid executor in dialog."));
}

/**
 * @brief Handles asynchronous connect errors.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog to get the executor from.
 */
template <Multiplexer Mux>
auto handle_connect_error(const socket_dialog<Mux> &dialog) -> void {
  switch (int error = errno) {
  case EINPROGRESS:
  case EAGAIN:
  case EALREADY:
  case EISCONN:
    return;

  default:
    dialog.socket->set_error(error);
  }
}
} // namespace detail

/**
 * @brief Asynchronously accepts a new connection on a listening socket.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param address A span to store the address of the connecting socket.
 * @return A sender that completes with an optional pair containing the new
 * socket handle and its address.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] accept_t *ptr,
                const socket_dialog<Mux> &dialog,
                std::span<std::byte> address) -> decltype(auto) {
  using trigger = ::io::execution::execution_trigger;
  using detail::get_executor;

  auto executor = get_executor(dialog);

  return executor->set(dialog.socket, trigger::READ,
                       [socket = dialog.socket.get(), address] {
                         auto result = ::io::accept(*socket, address);
                         return (result.first == INVALID_SOCKET)
                                    ? std::optional<decltype(result)>{}
                                    : std::optional{std::move(result)};
                       });
}

/**
 * @brief Binds a socket to a local address.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param address The local address to bind to.
 * @return The result of the bind operation.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] bind_t *ptr, const socket_dialog<Mux> &dialog,
                std::span<const std::byte> address) -> decltype(auto) {
  return ::io::bind(*dialog.socket, address);
}

/**
 * @brief Asynchronously connects a socket to a remote address.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param address The remote address to connect to.
 * @return A sender that completes when the connection is established.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] connect_t *ptr,
                const socket_dialog<Mux> &dialog,
                std::span<const std::byte> address) -> decltype(auto) {

  using trigger = ::io::execution::execution_trigger;
  using namespace detail;

  auto executor = get_executor(dialog);

  if (::io::connect(*dialog.socket, address))
    handle_connect_error(dialog);

  return executor->set(dialog.socket, trigger::WRITE,
                       [] { return std::optional<int>{0}; });
}

/**
 * @brief Performs a control operation on a socket.
 * @tparam Mux The multiplexer type.
 * @tparam Args The types of the arguments for the fcntl operation.
 * @param dialog The socket dialog.
 * @param args The arguments for the fcntl operation.
 * @return The result of the fcntl operation.
 */
template <Multiplexer Mux, typename... Args>
auto tag_invoke([[maybe_unused]] fcntl_t *ptr, const socket_dialog<Mux> &dialog,
                Args &&...args) -> decltype(auto) {
  return ::io::fcntl(*dialog.socket, std::forward<Args>(args)...);
}

/**
 * @brief Gets the address of the peer connected to a socket.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param address A span to store the peer's address.
 * @return The result of the getpeername operation.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] getpeername_t *ptr,
                const socket_dialog<Mux> &dialog,
                std::span<std::byte> address) -> decltype(auto) {
  return ::io::getpeername(*dialog.socket, address);
}

/**
 * @brief Gets the local address of a socket.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param address A span to store the local address.
 * @return The result of the getsockname operation.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] getsockname_t *ptr,
                const socket_dialog<Mux> &dialog,
                std::span<std::byte> address) -> decltype(auto) {
  return ::io::getsockname(*dialog.socket, address);
}

/**
 * @brief Gets a socket option.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option A span to store the option value.
 * @return The result of the getsockopt operation.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] getsockopt_t *ptr,
                const socket_dialog<Mux> &dialog, int level, int optname,
                std::span<std::byte> option) -> decltype(auto) {
  return ::io::getsockopt(*dialog.socket, level, optname, option);
}

/**
 * @brief Asynchronously receives a message from a socket.
 * @tparam Mux The multiplexer type.
 * @tparam Message The message type.
 * @param dialog The socket dialog.
 * @param msg The message to receive into.
 * @param flags The message flags.
 * @return A future that will contain the number of bytes received, or an empty
 * optional on error.
 */
template <Multiplexer Mux, MessageLike Message>
auto tag_invoke([[maybe_unused]] recvmsg_t *ptr,
                const socket_dialog<Mux> &dialog, Message &msg,
                int flags) -> decltype(auto) {
  using trigger = ::io::execution::execution_trigger;
  using detail::get_executor;

  auto executor = get_executor(dialog);

  auto msghdr = static_cast<socket_message_type>(msg);
  return executor->set(dialog.socket, trigger::READ,
                       [=, socket = dialog.socket.get()] {
                         std::streamsize len =
                             ::io::recvmsg(*socket, msghdr, flags);
                         return (len < 0) ? std::optional<std::streamsize>{}
                                          : std::optional<std::streamsize>{len};
                       });
}

/**
 * @brief Asynchronously sends a message on a socket.
 * @tparam Mux The multiplexer type.
 * @tparam Message The message type.
 * @param dialog The socket dialog.
 * @param msg The message to send.
 * @param flags The message flags.
 * @return A future that will contain the number of bytes sent, or an empty
 * optional on error.
 */
template <Multiplexer Mux, MessageLike Message>
auto tag_invoke([[maybe_unused]] sendmsg_t *ptr,
                const socket_dialog<Mux> &dialog, const Message &msg,
                int flags) -> decltype(auto) {
  using trigger = ::io::execution::execution_trigger;
  using detail::get_executor;

  auto executor = get_executor(dialog);

  return executor->set(dialog.socket, trigger::WRITE,
                       [socket = dialog.socket.get(), msg, flags] {
                         std::streamsize len =
                             ::io::sendmsg(*socket, std::move(msg), flags);
                         return (len < 0) ? std::optional<std::streamsize>{}
                                          : std::optional<std::streamsize>{len};
                       });
}

/**
 * @brief Marks a socket as passive, that is, as a socket that will be used to
 * accept incoming connection requests.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param backlog The maximum length to which the queue of pending connections
 * for sockfd may grow.
 * @return 0 on success, or -1 on error.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] listen_t *ptr,
                const socket_dialog<Mux> &dialog, int backlog) -> int {
  return ::io::listen(*dialog.socket, backlog);
}

/**
 * @brief Sets a socket option.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param level The protocol level at which the option resides.
 * @param optname The option name.
 * @param option The option value.
 * @return 0 on success, or -1 on error.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] setsockopt_t *ptr,
                const socket_dialog<Mux> &dialog, int level, int optname,
                std::span<const std::byte> option) -> int {
  return ::io::setsockopt(*dialog.socket, level, optname, option);
}

/**
 * @brief Shuts down part of a full-duplex connection.
 * @tparam Mux The multiplexer type.
 * @param dialog The socket dialog.
 * @param how The type of shutdown.
 * @return 0 on success, or -1 on error.
 */
template <Multiplexer Mux>
auto tag_invoke([[maybe_unused]] shutdown_t *ptr,
                const socket_dialog<Mux> &dialog, int how) -> int {
  return ::io::shutdown(*dialog.socket, how);
}

} // namespace io::socket
#endif // IO_ASYNC_OPERATIONS_HPP
