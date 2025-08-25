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

#include <algorithm>
#include <iterator>

namespace io::execution {

template <typename T> static auto pop(std::queue<T> &queue) -> decltype(auto) {
  auto tmp = queue.front();
  queue.pop();
  return tmp;
}

static auto run_queue(std::queue<poll_task *> &queue) -> void {
  while (!queue.empty()) {
    auto *task = pop(queue);
    task->do_complete(task);
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

static auto
make_interest_list(std::list<poll_event> &list) -> std::vector<struct pollfd> {
  std::vector<struct pollfd> tmp;
  std::erase_if(
      list, [](const auto &event) { return event.pfd.revents == POLLNVAL; });
  tmp.reserve(list.size());
  for (const auto &event : list)
    tmp.push_back(event.pfd);
  return tmp;
}

static auto make_completion(struct pollfd pfd, std::list<poll_event> &list,
                            std::mutex &mtx) -> poll_completion {
  auto event_it = std::ranges::lower_bound(
      list, pfd.fd, {}, [](const poll_event &event) { return event.pfd.fd; });
  if (event_it != std::end(list) && event_it->pfd.fd == pfd.fd)
    return {.revents = pfd.revents, .event = &(*event_it), .mtx = &mtx};
  return {};
}

auto poll_completion::operator()() const noexcept -> void {
  using detail::lock_exec;
  if (!event)
    return;

  if (revents & POLLOUT) {
    auto write_queue = lock_exec(std::unique_lock{*mtx}, [&]() {
      event->pfd.events &= ~POLLOUT;
      return std::exchange(event->write_queue, {});
    });
    run_queue(write_queue);
  }

  if (revents & POLLIN) {
    auto read_queue = lock_exec(std::unique_lock{*mtx}, [&]() {
      event->pfd.events &= ~POLLIN;
      return std::exchange(event->read_queue, {});
    });
    run_queue(read_queue);
  }
}

auto poll_multiplexer::run_once_for(interval_type interval) -> size_type {
  using detail::lock_exec;
  auto list = lock_exec(std::unique_lock{mtx_},
                        [&]() { return make_interest_list(list_); });

  auto num = poll_(list, static_cast<int>(interval.count()));

  auto count = num;
  auto end = std::end(list);
  for (auto it = std::begin(list); count && it != end; ++it) {
    auto &pfd = *it;
    if (pfd.revents && count-- && !(pfd.revents & POLLNVAL)) {
      auto handle = lock_exec(std::unique_lock{mtx_}, [&]() {
        return make_completion(pfd, list_, mtx_);
      });
      handle();
    }
  }
  return num;
}

} // namespace io::execution
