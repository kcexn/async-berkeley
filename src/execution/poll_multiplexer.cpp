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
#include "poll_multiplexer.hpp"
#include "detail/lock_exec.hpp"

#include <iterator>

namespace io::execution {

template <typename T> static auto pop(std::queue<T> &queue) -> decltype(auto) {
  auto tmp = queue.front();
  queue.pop();
  return tmp;
}

static auto
run_queue(std::queue<poll_multiplexer::operation *> &queue) -> void {
  while (!queue.empty()) {
    auto *operation = pop(queue);
    operation->complete();
  }
}

static auto get_queues(poll_multiplexer::events_type &events,
                       int key) -> poll_multiplexer::op_queues {
  auto qit = events.find(key);
  if (qit == std::end(events))
    return {};
  auto tmp = qit->second;
  events.erase(qit);
  return tmp;
}

template <typename BeginIt, typename EndIt>
  requires std::forward_iterator<BeginIt> && std::forward_iterator<EndIt>
static auto handle_events(std::mutex &mtx,
                          poll_multiplexer::events_type &events, int count,
                          BeginIt begin, EndIt end) {
  using detail::lock_exec;

  for (auto it = begin; count && it != end; ++it) {
    auto &event = *it;
    if (event.revents && count--) {
      auto queues = lock_exec(std::unique_lock{mtx},
                              [&]() { return get_queues(events, event.fd); });

      if (event.revents & POLLIN)
        run_queue(queues.read_queue);

      if (event.revents & POLLOUT)
        run_queue(queues.write_queue);
    }
  }
}

static auto poll_(std::vector<struct pollfd> &list, int duration) -> int {
  if (list.empty())
    return 0;

  auto num = poll(list.data(), list.size(), duration);
  if (num < 0) {
    switch (errno) {
    case EINTR:
      return poll_(list, duration);
    default:
      throw_system_error(IO_ERROR_MESSAGE("poll failed."));
    }
  }
  return num;
}

auto poll_multiplexer::run_for(interval_type interval) -> size_type {
  using detail::lock_exec;

  auto list = lock_exec(std::unique_lock{mtx_}, [&]() {
    std::erase_if(interest_,
                  [](const auto &event) { return event.revents == POLLNVAL; });
    return std::vector<event_base>(std::cbegin(interest_),
                                   std::cend(interest_));
  });

  auto num = poll_(list, static_cast<int>(interval.count()));

  handle_events(mtx_, events_, num, std::cbegin(list), std::cend(list));
  return num;
}

} // namespace io::execution
