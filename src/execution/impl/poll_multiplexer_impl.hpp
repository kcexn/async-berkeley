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
#include <execution/poll_multiplexer.hpp>

#include <algorithm>

namespace io::execution {

template <typename Receiver, detail::Operation<poll_multiplexer::event_type> Fn>
auto poll_multiplexer::poll_op<Receiver, Fn>::complete() -> void {
  std::lock_guard lock{*mtx};

  std::streamsize len = func(*event);
  if (len >= 0) {
    stdexec::set_value(std::move(receiver), len);
  } else {
    stdexec::set_error(std::move(receiver), errno);
  }
}

template <typename Receiver, detail::Operation<poll_multiplexer::event_type> Fn>
auto poll_multiplexer::poll_op<Receiver, Fn>::start() -> void {
  std::lock_guard lock{*mtx};

  auto [qit, emplaced] = events->try_emplace(event->key());
  auto &queue = (event->events & POLLIN) ? qit->second.read_queue
                                         : qit->second.write_queue;
  queue.push(this);
}

template <detail::Operation<poll_multiplexer::event_type> Fn>
template <typename Receiver>
auto poll_multiplexer::poll_sender<Fn>::connect(Receiver receiver)
    -> poll_multiplexer::poll_op<Receiver, Fn> {
  std::lock_guard lock{*mtx};

  auto event_it =
      std::lower_bound(std::begin(*interest), std::end(*interest), event.fd,
                       [&](const auto &lhs, int rhs) { return lhs.fd < rhs; });

  if (event_it != std::end(*interest) && event_it->fd == event.key()) {
    event_it->events |= event.events;
  } else {
    event_it = interest->insert(event_it, event);
  }

  return {.receiver = std::move(receiver),
          .func = std::move(func),
          .event = &*event_it,
          .interest = interest,
          .events = events,
          .mtx = mtx};
}

template <detail::Operation<poll_multiplexer::event_type> Fn>
auto poll_multiplexer::submit(event_type event, Fn func) -> poll_sender<Fn> {
  return {.func = std::move(func),
          .event = event,
          .interest = &interest_,
          .events = &events_,
          .mtx = &mtx_};
}

// template <typename Tag, typename Socket, typename Msg, typename Flags>
// auto tag_invoke([[maybe_unused]] Tag tag, const Socket &sock, const Msg
// &msg,
//                 Flags flags) -> decltype(auto) {
//   return submit({.fd = sock.fd(), .events = POLLOUT}, [=](auto &event)
//   {
//     return ::io::sendmsg(sock, msg, flags);
//   });
// }

} // namespace io::execution
#endif // IO_POLL_MULTIPLEXER_IMPL_HPP
