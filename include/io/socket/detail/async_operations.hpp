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
#include "io/detail/customization.hpp"
#include "io/error.hpp"
#include "io/execution/detail/execution_trigger.hpp"
#include "io/socket/socket_dialog.hpp"
#include "io/socket/socket_handle.hpp"
#include "socket.hpp"

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
    -> std::shared_ptr<typename socket_dialog<Mux>::executor_type>
{
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
auto handle_connect_error(const socket_dialog<Mux> &dialog) -> void
{
  switch (int error = errno)
  {
    case EINPROGRESS:
    case EAGAIN:
    case EALREADY:
    case EISCONN:
      return;

    default:
      dialog.socket->set_error(error);
  }
}

/**
 * @brief A helper struct for ensuring fairness in scheduling asynchronous
 * operations.
 */
struct fairness {
  /// @brief The type of the counter.
  using counter_type = std::uint8_t;

  /**
   * @brief Returns a reference to a static atomic counter.
   * @return A reference to the counter.
   */
  static auto counter() -> std::atomic<counter_type> &
  {
    static std::atomic<counter_type> counter;
    return counter;
  }
};

/**
 * @brief Sets an error on the socket handle if the error is not a "would block"
 * error.
 * @param handle The socket handle.
 * @param error The error code.
 * @return `true` if the error was `EWOULDBLOCK` or `EAGAIN`, `false`
 * otherwise.
 */
inline auto set_error_if_not_blocked(::io::socket::socket_handle &handle,
                                     int error) -> bool
{
  if (error != EWOULDBLOCK && error != EAGAIN)
    handle.set_error(error);
  return error != EWOULDBLOCK && error != EAGAIN;
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
                std::span<std::byte> address) -> decltype(auto)
{
  using socket_dialog = socket_dialog<Mux>;
  using result_t = std::pair<socket_dialog, std::span<const std::byte>>;
  using callback = std::function<std::optional<result_t>()>;
  using enum io::execution::execution_trigger;
  using namespace detail;

  auto executor = get_executor(dialog);
  auto &socket = dialog.socket;

  if constexpr (Mux::template is_eager_v<accept_t>)
  {
    if (++fairness::counter())
    {
      auto [sock, addr] = ::io::accept(*socket, address);

      if (sock)
      {
        auto res = std::make_shared<result_t>(
            socket_dialog{executor, executor->push(std::move(sock))}, addr);
        return executor->set(socket, EAGER, callback([res = std::move(res)] {
                               return std::optional<result_t>{std::move(*res)};
                             }));
      }

      if (set_error_if_not_blocked(*socket, errno))
        return executor->set(socket, EAGER, callback{});
    }
  }

  return executor->set(
      socket, READ, callback([=, socket = socket.get()] {
        auto [sock, addr] = ::io::accept(*socket, address);
        return (sock) ? std::optional<result_t>(
                            {{executor, executor->push(std::move(sock))}, addr})
                      : std::nullopt;
      }));
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
                std::span<const std::byte> address) -> decltype(auto)
{
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
                std::span<const std::byte> address) -> decltype(auto)
{

  using enum io::execution::execution_trigger;
  using namespace detail;

  auto executor = get_executor(dialog);
  auto &socket = dialog.socket;

  if (::io::connect(*socket, address))
    handle_connect_error(dialog);

  return executor->set(socket, WRITE, [] { return std::optional<int>{0}; });
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
                Args &&...args) -> decltype(auto)
{
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
                std::span<std::byte> address) -> decltype(auto)
{
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
                std::span<std::byte> address) -> decltype(auto)
{
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
                std::span<std::byte> option) -> decltype(auto)
{
  return ::io::getsockopt(*dialog.socket, level, optname, option);
}

/**
 * @brief Asynchronously receives a message from a socket.
 * @tparam Mux The multiplexer type.
 * @tparam Message The message type.
 * @param dialog The socket dialog.
 * @param msg The message to receive into.
 * @param flags The message flags.
 * @return A sender that will contain the number of bytes received, or an empty
 * optional on error.
 */
template <Multiplexer Mux, MessageLike Message>
auto tag_invoke([[maybe_unused]] recvmsg_t *ptr,
                const socket_dialog<Mux> &dialog, Message &msg,
                int flags) -> decltype(auto)
{
  using result_t = std::streamsize;
  using callback = std::function<std::optional<result_t>()>;
  using enum io::execution::execution_trigger;
  using namespace detail;

  auto executor = get_executor(dialog);
  auto &socket = dialog.socket;

  if constexpr (Mux::template is_eager_v<recvmsg_t>)
  {
    if (++fairness::counter())
    {
      result_t len = ::io::recvmsg(*socket, msg, flags);

      if (len >= 0)
      {
        return executor->set(socket, EAGER, callback([len] {
                               return std::optional<result_t>{len};
                             }));
      }

      if (set_error_if_not_blocked(*socket, errno))
        return executor->set(socket, EAGER, callback{});
    }
  }

  auto msghdr = static_cast<socket_message_type>(msg);
  return executor->set(
      socket, READ, callback([=, socket = socket.get()] {
        std::streamsize len = ::io::recvmsg(*socket, msghdr, flags);
        return (len < 0) ? std::nullopt : std::optional<result_t>{len};
      }));
}

/**
 * @brief Asynchronously sends a message on a socket.
 * @tparam Mux The multiplexer type.
 * @tparam Message The message type.
 * @param dialog The socket dialog.
 * @param msg The message to send.
 * @param flags The message flags.
 * @return A sender that will contain the number of bytes sent, or an empty
 * optional on error.
 */
template <Multiplexer Mux, MessageLike Message>
auto tag_invoke([[maybe_unused]] sendmsg_t *ptr,
                const socket_dialog<Mux> &dialog, const Message &msg,
                int flags) -> decltype(auto)
{
  using result_t = std::streamsize;
  using callback = std::function<std::optional<result_t>()>;
  using enum io::execution::execution_trigger;
  using namespace detail;

  auto executor = get_executor(dialog);
  auto &socket = dialog.socket;

  if constexpr (Mux::template is_eager_v<sendmsg_t>)
  {
    if (++fairness::counter())
    {
      std::streamsize len = ::io::sendmsg(*socket, msg, flags | MSG_NOSIGNAL);

      if (len >= 0)
      {
        return executor->set(socket, EAGER, callback([len] {
                               return std::optional<result_t>{len};
                             }));
      }

      if (set_error_if_not_blocked(*socket, errno))
        return executor->set(socket, EAGER, callback{});
    }
  }

  return executor->set(
      socket, WRITE, callback([=, socket = socket.get()] {
        result_t len = ::io::sendmsg(*socket, msg, flags | MSG_NOSIGNAL);
        return (len < 0) ? std::nullopt : std::optional<result_t>{len};
      }));
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
                const socket_dialog<Mux> &dialog, int backlog) -> int
{
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
                std::span<const std::byte> option) -> int
{
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
                const socket_dialog<Mux> &dialog, int how) -> int
{
  return ::io::shutdown(*dialog.socket, how);
}

} // namespace io::socket
#endif // IO_ASYNC_OPERATIONS_HPP
