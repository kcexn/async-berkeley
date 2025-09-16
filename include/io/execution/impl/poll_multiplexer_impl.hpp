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
 * @file poll_multiplexer_impl.hpp
 * @brief This file implements the poll_multiplexer class.
 */
#pragma once
#ifndef IO_POLL_MULTIPLEXER_IMPL_HPP
#define IO_POLL_MULTIPLEXER_IMPL_HPP
#include "io/execution/detail/poll_multiplexer.hpp"
#include "io/execution/detail/utilities.hpp"
#include "io/socket/detail/socket.hpp"
#include "io/socket/socket_handle.hpp"

// Customization point forward declarations
namespace io {
struct accept_t;
struct recvmsg_t;
struct sendmsg_t;
} // namespace io

// Forward declarations used for testing.
#ifndef NDEBUG
namespace io::execution {
auto run_queue(std::queue<poll_multiplexer::task *> &queue) -> void;
auto handle_poll_error(int error) -> void;
auto poll_(std::vector<pollfd> list, int duration) -> std::vector<pollfd>;
auto set_error(::io::socket::socket_handle &socket) -> void;
auto prepare_handles(short revents, poll_multiplexer::demultiplexer &demux)
    -> std::vector<std::queue<poll_multiplexer::task *>>;
auto make_ready_queues(const std::vector<pollfd> &list,
                       std::map<int, poll_multiplexer::demultiplexer> &demux)
    -> std::vector<std::queue<poll_multiplexer::task *>>;
auto clear_events(const std::vector<pollfd> &events,
                  std::vector<pollfd> &list) -> void;
} // namespace io::execution
#endif // NDEBUG

namespace io::execution {

#if IO_EAGER_ACCEPT
template <> struct poll_t::is_eager_t<accept_t> : public std::true_type {};
#endif

#if IO_EAGER_RECV
template <> struct poll_t::is_eager_t<recvmsg_t> : public std::true_type {};
#endif

#if IO_EAGER_SEND
template <> struct poll_t::is_eager_t<sendmsg_t> : public std::true_type {};
#endif

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::state<Receiver>::complete(task *task_ptr)
    -> void
{
  auto *self = static_cast<state *>(task_ptr);

  if (auto error = self->socket->get_error())
    return stdexec::set_error(std::move(self->receiver), error.value());

  if (auto result = self->func())
    return stdexec::set_value(std::move(self->receiver), std::move(*result));

  return stdexec::set_error(std::move(self->receiver), errno);
}

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::state<Receiver>::start() noexcept -> void
{
  using enum execution_trigger;
  if (socket->get_error() || trigger == EAGER)
    return complete(this);

  std::lock_guard lock{*mtx};
  task::complete = state::complete;

  if (trigger == WRITE)
    demux->write_queue.push(this);

  if (trigger == READ)
    demux->read_queue.push(this);

  demux->socket = socket.get();
}

/**
 * @brief Updates or inserts an event into a list of events.
 * @param list The list of events.
 * @param event The event to update or insert.
 * @return A pointer to the updated or inserted event.
 */
inline auto update_or_insert_event(std::vector<pollfd> *list,
                                   const pollfd &event) -> poll_t::event_type *
{
  auto pos = std::ranges::lower_bound(
      *list, event.fd, {}, [](const auto &event) { return event.fd; });

  if (pos != std::end(*list) && pos->fd == event.fd)
  {
    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    pos->events |= event.events;
  }
  else
  {
    pos = list->insert(pos, event);
  }

  return &*pos;
}

/**
 * @brief Creates a pollfd structure for a given socket and trigger.
 * @tparam Socket The type of the socket.
 * @param socket The socket to create the poll event for.
 * @param trigger The execution trigger to wait for.
 * @return A pollfd structure.
 */
template <SocketLike Socket>
auto make_poll_event(const Socket &socket,
                     execution_trigger trigger) -> struct pollfd {
  using socket_t = socket::native_socket_type;
  using enum execution_trigger;

  struct pollfd event = {.fd = static_cast<socket_t>(socket)};

  if (trigger == READ)
    event.events |= POLLIN;

  if (trigger == WRITE)
    event.events |= POLLOUT;

  return event;
}

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::connect(Receiver &&receiver)
    -> state<std::decay_t<Receiver>>
{
  using enum execution_trigger;
  if (!socket->get_error() && trigger != EAGER)
  {
    with_lock(std::unique_lock{*mtx}, [&] {
      update_or_insert_event(list, make_poll_event(*socket, trigger));
    });
  }

  return {.socket = std::move(socket),
          .func = std::move(func),
          .demux = demux,
          .mtx = mtx,
          .receiver = std::forward<Receiver>(receiver),
          .trigger = trigger};
}

template <Completion Fn>
// NOLINTNEXTLINE(performance-unnecessary-value-param)
auto poll_multiplexer::set(std::shared_ptr<::io::socket::socket_handle> socket,
                           execution_trigger trigger,
                           Fn &&func) -> sender<std::decay_t<Fn>>
{
  using socket_t = ::io::socket::native_socket_type;

  auto key = static_cast<socket_t>(*socket);
  auto [demux_it, emplaced] = demux_.try_emplace(key);

  return {.socket = std::move(socket),
          .func = std::forward<Fn>(func),
          .trigger = trigger,
          .demux = &(demux_it->second),
          .list = &list_,
          .mtx = &mtx_};
}
} // namespace io::execution
#endif // IO_POLL_MULTIPLEXER_IMPL_HPP
