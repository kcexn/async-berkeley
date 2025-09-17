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
#include "io/execution/detail/poll_multiplexer.hpp"
#include "io/error.hpp"
#include "io/execution/detail/utilities.hpp"
#include "io/macros.h"
#include "io/socket/socket_handle.hpp"

#include <sys/socket.h>

namespace io::execution {
namespace {
/**
 * @brief Pops an element from a queue and returns it.
 * @tparam T The type of the elements in the queue.
 * @param queue The queue to pop from.
 * @return The popped element.
 */
template <typename T> auto pop(std::queue<T> &queue) -> decltype(auto)
{
  auto tmp = queue.front();
  queue.pop();
  return tmp;
}

/**
 * @brief Filters a range by removing elements that satisfy a predicate.
 * @tparam R The type of the range.
 * @tparam P The type of the predicate.
 * @param list The range to filter.
 * @param pred The predicate to use for filtering.
 * @return The filtered range.
 */
template <std::ranges::range R, std::predicate<typename R::value_type> P>
auto filter(R list, P &&pred) -> decltype(auto)
{
  auto [begin, end] = std::ranges::remove_if(list, std::forward<P>(pred));
  list.erase(begin, end);
  return list;
}
} // namespace

/**
 * @brief Executes all tasks in a queue.
 * @param queue The queue of tasks to execute.
 */
IO_STATIC(auto) run_queue(std::queue<poll_multiplexer::task *> &queue) -> void
{
  while (!queue.empty())
  {
    auto *task = pop(queue);
    task->execute();
  }
}

/**
 * @brief Handles errors from the poll system call.
 * @param error The error code to handle.
 */
IO_STATIC(auto) handle_poll_error(int error) -> void
{
  if (error != EINTR)
    throw_system_error(IO_ERROR_MESSAGE("poll failed."));
}

/**
 * @brief A wrapper around the poll system call.
 * @param list The list of file descriptors to poll.
 * @param duration The timeout for the poll call.
 * @return The list of file descriptors that have events.
 */
IO_STATIC(auto)
poll_(std::vector<pollfd> list, int duration)->std::vector<pollfd>
{
  if (list.empty())
    return list;

  int num = 0;
  while ((num = poll(list.data(), list.size(), duration)) < 0)
  {
    handle_poll_error(errno); // GCOVR_EXCL_LINE
  }

  return filter(std::move(list),
                [](const auto &event) { return event.revents == 0; });
}

/**
 * @brief Gets the socket error and sets it on the socket handle.
 * @param socket The socket handle to set the error on.
 */
IO_STATIC(auto)
set_error(::io::socket::socket_handle &socket)->void
{
  using native_socket_type = ::io::socket::native_socket_type;
  using socklen_type = ::io::socket::socklen_type;

  int error = 0;
  socklen_type len = sizeof(error);
  if (::getsockopt(static_cast<native_socket_type>(socket), SOL_SOCKET,
                   SO_ERROR, &error, &len))
  {
    switch (error = errno)
    {
      case EBADF:
      case ENOTSOCK:
        break;

      default:                                       // GCOVR_EXCL_LINE
        throw_system_error(                          // GCOVR_EXCL_LINE
            IO_ERROR_MESSAGE("getsockopt failed.")); // GCOVR_EXCL_LINE
    }
  }
  socket.set_error(error);
}

/**
 * @brief Prepares the read and write queues for a demultiplexer based on the
 * returned events.
 * @param revents The returned events from the poll call.
 * @param demux The demultiplexer to prepare.
 * @return A vector of queues of tasks to be executed.
 */
IO_STATIC(auto)
prepare_handles(short revents, poll_multiplexer::demultiplexer &demux)
    ->std::vector<std::queue<poll_multiplexer::task *>>
{
  std::array<std::queue<poll_multiplexer::task *>, 2> tmp{};

  /* A POLLNVAL condition can be returned on the socket
   * if the user of the library has statically cast
   * the socket_handle to a native_socket_type and
   * subsequently called `close`. Under normal
   * circumstances, it is impossible for a socket
   * to be invalid after the sender has been constructed
   * constructed as the sender takes shared ownership
   * of the underlying socket.
   */
  if (revents & (POLLERR | POLLNVAL))
    set_error(*demux.socket);

  // NOLINTNEXTLINE(readability-qualified-auto)
  auto pos = std::begin(tmp);
  if ((revents & (POLLOUT | POLLERR | POLLNVAL)) &&
      !(demux.write_queue.empty()))
    *(pos++) = std::exchange(demux.write_queue, {});

  if ((revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) &&
      !(demux.read_queue.empty()))
    *(pos++) = std::exchange(demux.read_queue, {});

  return {std::begin(tmp), pos};
}

/**
 * @brief Copies a list of pollfds and clears the events in the original list.
 * @param list The list of pollfds to copy and clear.
 * @return A copy of the list.
 */
static auto copy_active(std::vector<pollfd> &list) -> std::vector<pollfd>
{
  std::vector<pollfd> tmp{};
  tmp.reserve(list.size());

  std::ranges::copy_if(list, std::back_inserter(tmp),
                       [](const auto &event) { return event.events; });

  return tmp;
} // GCOVR_EXCL_LINE

/**
 * @brief Clears events that will be handled.
 * @param events The list of events to be handled returned by poll_
 * @param list The interest list managed by the poll_multiplexer.
 */
IO_STATIC(auto)
clear_events(const std::vector<pollfd> &events, std::vector<pollfd> &list)->void
{
  for (const auto &event : events)
  {
    auto pfd = std::ranges::lower_bound(list, event.fd, {},
                                        [](const auto &tmp) { return tmp.fd; });
    if (pfd != std::end(list) && pfd->fd == event.fd)
    {
      if (event.revents & (POLLERR | POLLNVAL))
        pfd->events = 0;

      // NOLINTNEXTLINE(bugprone-narrowing-conversions)
      pfd->events &= ~(event.revents);
    }
  }
}

/**
 * @brief Creates a vector of ready queues from a list of pollfds.
 * @param list The list of pollfds that have events.
 * @param demux The map of demultiplexers.
 * @return A vector of queues of tasks to be executed.
 */
IO_STATIC(auto)
make_ready_queues(const std::vector<pollfd> &list,
                  std::map<int, poll_multiplexer::demultiplexer> &demux)
    ->std::vector<std::queue<poll_multiplexer::task *>>
{
  std::vector<std::queue<poll_multiplexer::task *>> tmp{};
  tmp.reserve(2 * list.size());

  for (const auto &event : list)
  {
    auto demux_it = demux.find(event.fd);
    if (demux_it != std::end(demux))
    {
      auto prepared = prepare_handles(event.revents, demux_it->second);
      tmp.insert(std::end(tmp), std::begin(prepared), std::end(prepared));
    }
  }

  return tmp;
} // GCOVR_EXCL_LINE

auto poll_multiplexer::wait_for(interval_type interval) -> size_type
{
  auto list =
      with_lock(std::unique_lock{mtx_}, [&] { return copy_active(list_); });

  list = poll_(std::move(list), static_cast<int>(interval.count()));

  with_lock(std::unique_lock{mtx_}, [&] { clear_events(list, list_); });

  auto ready_queues = with_lock(
      std::unique_lock{mtx_}, [&] { return make_ready_queues(list, demux_); });

  for (auto &queue : ready_queues)
  {
    run_queue(queue);
  }

  return list.size();
}

} // namespace io::execution
