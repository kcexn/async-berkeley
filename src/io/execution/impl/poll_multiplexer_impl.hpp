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
#include <boost/predef.h>
#if BOOST_OS_WINDOWS
#include "io/socket/platforms/windows/socket.hpp"
#else
#include "io/socket/platforms/posix/socket.hpp"
#endif
#include "io/execution/detail/utilities.hpp"
#include "io/execution/poll_multiplexer.hpp"
#include "io/socket/socket_handle.hpp"

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
} // namespace io::execution
#endif // NDEBUG

namespace io::execution {

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::state<Receiver>::complete(task *task_ptr)
    -> void {
  auto *self = static_cast<state *>(task_ptr);

  if (auto error = self->socket->get_error())
    return stdexec::set_error(std::move(self->receiver), error.value());

  if (auto result = self->func())
    return stdexec::set_value(std::move(self->receiver), std::move(*result));

  return stdexec::set_error(std::move(self->receiver), errno);
}

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::state<Receiver>::start() noexcept -> void {
  if (auto error = socket->get_error())
    return stdexec::set_error(std::move(receiver), error.value());

  std::lock_guard lock{*mtx};
  task::complete = state::complete;
  if (mask & POLLOUT) {
    demux->write_queue.push(this);
  } else if (mask & POLLIN) {
    demux->read_queue.push(this);
  }
  demux->socket = socket.get();
}

/**
 * @brief Updates or inserts an event into a list of events.
 * @param list The list of events.
 * @param event The event to update or insert.
 * @return A pointer to the updated or inserted event.
 */
inline auto
update_or_insert_event(std::vector<pollfd> *list,
                       const pollfd &event) -> poll_t::event_type * {
  auto pos = std::ranges::lower_bound(
      *list, event.fd, {}, [](const auto &event) { return event.fd; });

  if (pos != std::end(*list) && pos->fd == event.fd) {
    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    pos->events |= event.events;
  } else {
    pos = list->insert(pos, event);
  }

  return &*pos;
}

template <Completion Fn>
template <typename Receiver>
auto poll_multiplexer::sender<Fn>::connect(Receiver &&receiver)
    -> state<std::decay_t<Receiver>> {
  if (!socket->get_error()) {
    with_lock(std::unique_lock{*mtx},
              [&] { update_or_insert_event(list, event); });
  }

  return {.socket = std::move(socket),
          .func = std::move(func),
          .demux = demux,
          .mtx = mtx,
          .receiver = std::forward<Receiver>(receiver),
          .mask = event.events};
}

template <Completion Fn>
// NOLINTNEXTLINE(performance-unnecessary-value-param)
auto poll_multiplexer::set(std::shared_ptr<::io::socket::socket_handle> socket,
                           execution_trigger trigger,
                           Fn &&func) -> sender<std::decay_t<Fn>> {
  using native_socket_type = ::io::socket::native_socket_type;

  auto key = static_cast<native_socket_type>(*socket);
  auto [demux_it, emplaced] = demux_.try_emplace(key);

  auto flags = static_cast<std::uint8_t>(trigger);
  struct pollfd event {
    .fd = key
  };

  if (flags & static_cast<std::uint8_t>(execution_trigger::READ))
    event.events |= POLLIN;

  if (flags & static_cast<std::uint8_t>(execution_trigger::WRITE))
    event.events |= POLLOUT;

  return {.socket = std::move(socket),
          .func = std::forward<Fn>(func),
          .event = event,
          .demux = &(demux_it->second),
          .list = &list_,
          .mtx = &mtx_};
}
} // namespace io::execution
#endif // IO_POLL_MULTIPLEXER_IMPL_HPP
