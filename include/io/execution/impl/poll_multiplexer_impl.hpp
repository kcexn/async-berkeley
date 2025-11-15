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
 * @details This file contains the implementation of the poll_multiplexer class,
 * which is a concrete implementation of the multiplexer concept using the
 * `poll` system call.
 */
#pragma once
#ifndef IO_POLL_MULTIPLEXER_IMPL_HPP
#define IO_POLL_MULTIPLEXER_IMPL_HPP
#include "io/error.hpp"
#include "io/execution/detail/utilities.hpp"
#include "io/execution/poll_multiplexer.hpp"
#include "io/socket/detail/socket.hpp"
#include "io/socket/socket_handle.hpp"
#include "io/socket/socket_option.hpp"

#include <algorithm>
#include <system_error>
// Customization point forward declarations
namespace io {
struct accept_t;
struct recvmsg_t;
struct sendmsg_t;
} // namespace io

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

/**
 * @brief Completes the operation and sends the result to the receiver.
 * @details This function is called when the operation is complete. It gets the
 * result of the operation and sends it to the receiver. If the operation
 * failed, it sends the error to the receiver.
 * @param task_ptr A pointer to the task to complete.
 */
template <AllocatorLike Allocator>
template <Completion Fn>
template <typename Receiver>
auto basic_poll_multiplexer<Allocator>::sender<Fn>::state<Receiver>::complete(
    task *task_ptr) noexcept -> void
{
  auto *self = static_cast<state *>(task_ptr);

  auto error = self->socket->get_error();
  if (error && error != std::errc::operation_would_block)
    return stdexec::set_error(std::move(self->receiver), error);

  if (auto result = self->func())
    return stdexec::set_value(std::move(self->receiver), std::move(*result));

  return stdexec::set_error(std::move(self->receiver),
                            std::error_code{errno, std::system_category()});
}

/**
 * @brief Starts the operation.
 * @details This function is called to start the operation. If the operation can
 * be completed eagerly, it is completed immediately. Otherwise, it is added to
 * the appropriate queue to be completed later.
 */
template <AllocatorLike Allocator>
template <Completion Fn>
template <typename Receiver>
auto basic_poll_multiplexer<Allocator>::sender<Fn>::state<
    Receiver>::start() noexcept -> void
{
  using enum execution_trigger;
  auto error = socket->get_error();
  if (trigger == EAGER || (error && error != std::errc::operation_would_block))
    return complete(this);

  std::lock_guard lock{*mtx};
  task::tail = state::complete;

  if (trigger == WRITE)
    demux->write_queue.push(this);

  if (trigger == READ)
    demux->read_queue.push(this);

  demux->socket = socket.get();
}

/**
 * @brief Updates or inserts an event into a list of events.
 * @details This function searches for an event with the same file descriptor
 * in the list. If it finds one, it updates the event's interest set. Otherwise,
 * it inserts a new event into the list. The list is kept sorted by file
 * descriptor.
 * @param list The list of events.
 * @param event The event to update or insert.
 * @return A pointer to the updated or inserted event.
 */
template <AllocatorLike Allocator>
auto update_or_insert_event(std::vector<pollfd, Allocator> *list,
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
 * @details This function creates a pollfd structure for a given socket and
 * trigger. The pollfd structure is used to specify the events to wait for on a
 * file descriptor.
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

/**
 * @brief Connects a sender to a receiver.
 * @details This function is called to connect a sender to a receiver. It
 * creates a state object that will be used to manage the operation.
 * @param receiver The receiver to connect to.
 * @return The state object for the operation.
 */
template <AllocatorLike Allocator>
template <Completion Fn>
template <typename Receiver>
auto basic_poll_multiplexer<Allocator>::sender<Fn>::connect(Receiver &&receiver)
    -> state<std::decay_t<Receiver>>
{
  using socket_type = socket::native_socket_type;
  using enum execution_trigger;

  demultiplexer *demux_ptr = nullptr;
  auto error = socket->get_error();
  if ((!error || error == std::errc::operation_would_block) && trigger != EAGER)
  {
    std::lock_guard lock{*mtx};

    auto sockfd = static_cast<socket_type>(*socket);
    if (demux->size() < static_cast<std::size_t>(sockfd) + 1)
      demux->resize(sockfd + 1);

    demux_ptr = std::addressof(demux->at(sockfd));

    update_or_insert_event(list, make_poll_event(*socket, trigger));
  }

  return {.func = std::move(func),
          .socket = std::move(socket),
          .receiver = std::forward<Receiver>(receiver),
          .demux = demux_ptr,
          .mtx = mtx,
          .trigger = trigger};
}

/**
 * @brief Creates a sender for an operation.
 * @details This function is called to create a sender for an operation. The
 * sender will be used to connect to a receiver and start the operation.
 * @param socket The socket to perform the operation on.
 * @param trigger The execution trigger to wait for.
 * @param func The function to execute when the operation is ready.
 * @return A sender for the operation.
 */
template <AllocatorLike Allocator>
template <Completion Fn>
auto basic_poll_multiplexer<Allocator>::set(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    std::shared_ptr<socket_handle> socket, execution_trigger trigger,
    Fn &&func) -> sender<std::decay_t<Fn>>
{
  return {.func = std::forward<Fn>(func),
          .socket = std::move(socket),
          .demux = &demux_,
          .list = &list_,
          .mtx = &mtx_,
          .trigger = trigger};
}

/**
 * @brief Executes all tasks in a queue.
 * @param queue The queue of tasks to execute.
 */
template <AllocatorLike Allocator>
auto run_queue(typename basic_poll_multiplexer<Allocator>::intrusive_task_queue
                   &queue) -> void
{
  while (!queue.is_empty())
  {
    auto *task = queue.pop();
    task->execute();
  }
}

/**
 * @brief Handles errors from the poll system call.
 * @details This function is called when the poll system call returns an error.
 * It throws a system_error if the error is not an interrupt.
 * @param error The error code to handle.
 */
inline auto handle_poll_error(const std::error_code &error) -> void
{
  if (error != std::errc::interrupted)
    throw_system_error(IO_ERROR_MESSAGE("poll failed."));
}

namespace detail {

/**
 * @brief Calculates the remaining duration from a start time and updates the
 * start time.
 *
 * This function measures the time elapsed since the `start` time_point,
 * subtracts it from the initial `duration`, and returns the remaining time. It
 * also updates the `start` time_point to the current time.
 *
 * @tparam Clock The clock type used for the time_point.
 * @tparam Duration The duration type used for the time_point.
 * @param duration The initial duration in milliseconds.
 * @param[in,out] start The time_point marking the beginning of the duration.
 * This parameter is updated to the current time (`Clock::now()`) on each call.
 * @return The remaining time in milliseconds, guaranteed to be non-negative.
 */
template <typename Clock, typename Duration>
auto remaining_duration(int duration,
                        std::chrono::time_point<Clock, Duration> &start) -> int
{
  using clock = std::decay_t<decltype(start)>::clock;
  static constexpr auto to_milliseconds = [](const auto &duration) -> int {
    using namespace std::chrono;

    return duration_cast<milliseconds>(duration).count();
  };

  auto old_start = start;
  start = clock::now();
  return std::max(0, duration - to_milliseconds(start - old_start));
}

} // namespace detail

/**
 * @brief A wrapper around the poll system call.
 * @details This function calls the poll system call and handles any errors. It
 * also removes any file descriptors from the list that do not have any events.
 * @param list The list of file descriptors to poll.
 * @param duration The timeout for the poll call.
 * @return The list of file descriptors that have events.
 */
template <AllocatorLike Allocator>
auto poll_(std::vector<pollfd, Allocator> list,
           int duration) -> std::vector<pollfd>
{
  using namespace detail;
  using clock = std::chrono::steady_clock;

  if (list.empty())
    return list;

  auto start = clock::now();
  while (poll(list.data(), list.size(), duration) < 0)
  {
    handle_poll_error({errno, std::system_category()});
    if (duration > -1)
      duration = remaining_duration(duration, start);
  }

  auto [first, last] = std::ranges::remove_if(
      list, [](const auto &event) { return event.revents == 0; });
  list.erase(first, last);
  return list;
}

/**
 * @brief Checks if the error is recoverable.
 *
 * This function checks if the error set by getsockopt is a recoverable
 * one. If it is recoverable, return the value. Otherwise throw an exception.
 *
 * @param error The error code to test.
 * @returns The error code to set on the socket.
 */
inline auto handle_getsockopt_error(std::error_code error) -> int
{
  if (error != std::errc::bad_file_descriptor &&
      error != std::errc::not_a_socket)
  {
    throw_system_error(IO_ERROR_MESSAGE("getsockopt failed."));
  }

  return error.value();
}

/**
 * @brief Gets the socket error and sets it on the socket handle.
 * @details This function gets the value of the SO_ERROR socket option and sets
 * it on the socket handle. This is used to get the error that occurred during
 * an asynchronous operation.
 * @param socket The socket handle to set the error on.
 */
inline auto set_error(::io::socket::socket_handle &socket) -> void
{
  using socket_option = ::io::socket::socket_option<int>;

  socket_option error{0};
  auto [ret, optval] = ::io::getsockopt(socket, SOL_SOCKET, SO_ERROR, error);
  if (ret)
    *error = handle_getsockopt_error({errno, std::system_category()});
  socket.set_error(*error);
}

/**
 * @brief Moves tasks from the demultiplexer's read and write queues to the
 * ready queue based on poll events.
 * @param revents The events returned from a poll call.
 * @param demux The demultiplexer containing the read and write task queues.
 * @param ready The queue to which ready tasks will be moved.
 */
template <AllocatorLike Allocator>
auto prepare_handles(
    short revents,
    typename basic_poll_multiplexer<Allocator>::demultiplexer &demux,
    typename basic_poll_multiplexer<Allocator>::intrusive_task_queue &ready)
    -> void
{
  /**
   * A POLLNVAL condition can be returned on the socket
   * if the user of the library has statically cast
   * the socket_handle to a native_socket_type and
   * subsequently called `close`. Under normal
   * circumstances, it is impossible for a socket
   * to be invalid after the sender has been constructed
   * constructed as the sender takes shared ownership
   * of the underlying socket.
   */
  if (revents & (POLLERR | POLLNVAL))
    set_error(*demux.socket); // GCOVR_EXCL_LINE

  if (revents & (POLLOUT | POLLERR | POLLNVAL))
    ready.move_back(std::move(demux.write_queue));

  if (revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL))
    ready.move_back(std::move(demux.read_queue));
}

/**
 * @brief Copies a list of pollfds and clears the events in the original list.
 * @param list The list of pollfds to copy and clear.
 * @return A copy of the list.
 */
template <AllocatorLike Allocator>
auto copy_active(std::vector<pollfd, Allocator> &list)
    -> std::vector<pollfd, Allocator>
{
  std::vector<pollfd, Allocator> tmp{list.get_allocator()};
  tmp.reserve(list.size());

  std::ranges::copy_if(list, std::back_inserter(tmp),
                       [](const auto &event) { return event.events; });

  return tmp;
} // GCOVR_EXCL_LINE

/**
 * @brief Clears events that will be handled.
 * @details This function clears the events in the interest list that will be
 * handled. This is done to prevent the same event from being handled multiple
 * times.
 * @param events The list of events to be handled returned by poll_
 * @param list The interest list managed by the poll_multiplexer.
 */
template <AllocatorLike Allocator>
auto clear_event(const struct pollfd &event,
                 std::vector<pollfd, Allocator> &list) -> void
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

/**
 * @brief Waits for events on the file descriptors in the interest list.
 * @details This function is the main entry point for the poll_multiplexer. It
 * waits for events on the file descriptors in the interest list and then
 * executes the corresponding tasks.
 * @param interval The maximum time to wait for an event.
 * @return The number of events that were handled.
 */
template <AllocatorLike Allocator>
auto basic_poll_multiplexer<Allocator>::wait_for(interval_type interval)
    -> size_type
{
  auto list = with_lock(mtx_, [&] { return copy_active(list_); });

  list = poll_(std::move(list), static_cast<int>(interval.count()));

  intrusive_task_queue ready_queue;

  with_lock(mtx_, [&] {
    for (const auto &event : list)
    {
      clear_event(event, list_);
      prepare_handles<Allocator>(event.revents, demux_[event.fd], ready_queue);
    }
  });

  run_queue<Allocator>(ready_queue);

  return list.size();
}

/**
 * @brief Constructs a basic_poll_multiplexer.
 * @param alloc The allocator to use for all allocations.
 */
template <AllocatorLike Allocator>
constexpr basic_poll_multiplexer<Allocator>::basic_poll_multiplexer(
    const Allocator &alloc) noexcept(noexcept(Allocator()))
    : demux_{alloc}, list_{alloc} {};

} // namespace io::execution
#endif // IO_POLL_MULTIPLEXER_IMPL_HPP
