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
#include "detail/utilities.hpp"
#include <io/error.hpp>
#include <io/macros.h>

namespace io::execution {
namespace {
template <typename T> auto pop(std::queue<T> &queue) -> decltype(auto) {
  auto tmp = queue.front();
  queue.pop();
  return tmp;
}

template <typename T, typename Predicate>
  requires std::is_invocable_v<Predicate, const T &>
auto filter(std::vector<T> list, Predicate &&pred) -> std::vector<T> {
  auto [begin, end] =
      std::ranges::remove_if(list, std::forward<Predicate>(pred));
  list.erase(begin, end);
  return list;
}
} // namespace

IO_STATIC(auto) run_queue(std::queue<poll_multiplexer::task *> &queue) -> void {
  while (!queue.empty()) {
    auto *task = pop(queue);
    task->execute();
  }
}

IO_STATIC(auto) handle_poll_error(int err) -> void {
  if (err != EINTR)
    throw_system_error(IO_ERROR_MESSAGE("poll failed."));
}

IO_STATIC(auto)
poll_(std::vector<pollfd> list, int duration)->std::vector<pollfd> {
  if (list.empty())
    return list;

  int num = 0;
  while ((num = poll(list.data(), list.size(), duration)) < 0) {
    handle_poll_error(errno);
  }

  return filter(std::move(list), [](const auto &event) {
    return (event.revents & POLLNVAL) || (event.revents == 0);
  });
}

auto poll_multiplexer::wait_for(interval_type interval) -> size_type {
  auto list = with_lock(std::unique_lock{mtx_}, [&] {
    auto tmp = list_;

    for (auto &event : list_) {
      event.events = 0;
    }

    return tmp;
  });

  list = poll_(std::move(list), static_cast<int>(interval.count()));

  auto ready_queues = with_lock(std::unique_lock{mtx_}, [&] {
    std::vector<std::queue<task *>> tmp{};
    tmp.reserve(2 * list.size());

    for (const auto &event : list) {
      auto demux_it = demux_.find(event.fd);
      if (demux_it != std::end(demux_)) {
        auto &demux = demux_it->second;

        if ((event.revents & (POLLOUT | POLLERR)) &&
            !(demux.write_queue.empty())) {
          tmp.push_back(std::exchange(demux.write_queue, {}));
        }

        if ((event.revents & (POLLIN | POLLHUP | POLLERR)) &&
            !(demux.read_queue.empty())) {
          tmp.push_back(std::exchange(demux.read_queue, {}));
        }
      }
    }

    return tmp;
  });

  for (auto &queue : ready_queues) {
    run_queue(queue);
  }

  return list.size();
}

} // namespace io::execution
