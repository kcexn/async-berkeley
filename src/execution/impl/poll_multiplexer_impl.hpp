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
#ifndef IO_POLL_MULTIPLEXER_IMPL_HPP
#define IO_POLL_MULTIPLEXER_IMPL_HPP
#include <execution/detail/lock_exec.hpp>
#include <execution/poll_multiplexer.hpp>

#include <algorithm>

namespace io::execution {

template <typename Receiver, typename Fn>
  requires std::is_invocable_v<Fn, pollfd *>
auto poll_multiplexer::poll_op<Receiver, Fn>::complete(poll_task *task) noexcept
    -> void {
  auto *self = static_cast<poll_op *>(task);
  std::streamsize len = self->func(self->event);
  if (len >= 0) {
    stdexec::set_value(std::move(self->receiver), len);
  } else {
    stdexec::set_error(std::move(self->receiver), errno);
  }
}

template <typename Receiver, typename Fn>
  requires std::is_invocable_v<Fn, pollfd *>
auto poll_multiplexer::poll_op<Receiver, Fn>::start() noexcept -> void {
  std::lock_guard lock{*mtx};

  poll_task::do_complete = poll_op::complete;
  event->pfd.events |= events_mask;

  if (events_mask & POLLIN)
    event->read_queue.push(this);

  if (events_mask & POLLOUT)
    event->write_queue.push(this);
}

inline auto update_or_insert_event(std::list<poll_event> *list,
                                   const poll_event &event) -> decltype(auto) {
  auto event_it = std::ranges::lower_bound(
      *list, event.key(), {}, [](const auto &event) { return event.pfd.fd; });

  if (event_it != std::end(*list) && event_it->pfd.fd == event.key()) {
    // NOLINTNEXTLINE
    event_it->pfd.events |= static_cast<short>(event.pfd.events);
  } else {
    event_it = list->insert(event_it, event);
  }

  return event_it;
}

template <typename Fn>
  requires std::is_invocable_v<Fn, pollfd *>
template <typename Receiver>
auto poll_multiplexer::poll_sender<Fn>::connect(Receiver receiver)
    -> poll_op<Receiver, Fn> {
  using detail::lock_exec;

  auto event_it = lock_exec(std::unique_lock{*mtx}, [&]() {
    return update_or_insert_event(list, event);
  });

  return {.receiver = std::move(receiver),
          .func = std::move(func),
          .events_mask = event.pfd.events,
          .event = &(*event_it),
          .mtx = mtx};
}

template <typename Fn>
  requires std::is_invocable_v<Fn, pollfd *>
auto poll_multiplexer::submit(event_type event, Fn func) -> poll_sender<Fn> {
  return {.func = std::move(func),
          .event = {.pfd = std::move(event)},
          .list = &list_,
          .mtx = &mtx_};
}

} // namespace io::execution

#endif // IO_POLL_MULTIPLEXER_IMPL_HPP
