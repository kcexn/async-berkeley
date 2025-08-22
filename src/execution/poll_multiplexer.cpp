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

namespace io::execution {

auto poll_multiplexer::run_for(interval_type interval) -> size_type {
  auto list = lock_exec(std::unique_lock{mtx_}, [&]() {
    return std::vector<event_base>(std::cbegin(interest_),
                                   std::cend(interest_));
  });

  auto num_events =
      poll(list.data(), list.size(), static_cast<int>(interval.count()));
  if (num_events < 0)
    throw_system_error(IO_ERROR_MESSAGE("poll failed."));

  auto end = std::end(list);
  for (auto it = std::begin(list); num_events && it != end; ++it) {
    auto &event = *it;
    if (event.revents && num_events--) {
      auto queues = lock_exec(std::unique_lock{mtx_}, [&]() {
        auto qit = events_.find(event.fd);
        if (qit == std::end(events_))
          return op_queues_{};
        auto tmp = qit->second;
        events_.erase(qit);
        return tmp;
      });

      if (event.revents & POLLIN) {
        auto &queue = queues.read_queue;

        while (!queue.empty()) {
          auto *operation = lock_exec(std::unique_lock{mtx_}, [&]() {
            auto *tmp = queue.front();
            queue.pop();
            return tmp;
          });

          operation->complete();
        }
      }

      if (event.revents & POLLOUT) {
        auto &queue = queues.write_queue;

        while (!queue.empty()) {
          auto *operation = lock_exec(std::unique_lock{mtx_}, [&]() {
            auto *tmp = queue.front();
            queue.pop();
            return tmp;
          });

          operation->complete();
        }
      }
    }
  }

  // TODO: need to think of a way to clean the interest list.

  return num_events;
}

} // namespace io::execution
