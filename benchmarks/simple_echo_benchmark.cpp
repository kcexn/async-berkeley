/* Copyright 2025 Kevin Exton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * you may obtain a copy of the License at
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
 * @file simple_echo_benchmark.cpp
 * @brief A simple benchmark for async-berkeley.
 */
// NOLINTBEGIN
#include <benchmark/benchmark.h>
#include <io.hpp>

#include <cassert>
#include <iostream>

#include <arpa/inet.h>
#include <sys/socket.h>

// Using declarations for brevity
using namespace exec;

// Type aliases for the specific implementations used in this benchmark.
using multiplexer = ::io::execution::poll_multiplexer;
using socket_type = ::io::socket::native_socket_type;
using basic_triggers = ::io::execution::basic_triggers<multiplexer>;
using socket_dialog = ::io::socket::socket_dialog<multiplexer>;
using message_buffer = std::string;
using socket_message = ::io::socket::socket_message<sockaddr_in>;

static const std::size_t NUM_ECHOES = 10000;
static auto count = std::atomic<std::size_t>{0};
static const auto message = std::string(1280UL, 'x');
static auto read_buffer = std::vector<std::byte>(message.size());

/**
 * @brief A simple error handler for asynchronous operations.
 *
 * This function is called when an error occurs in a sender chain. It prints
 * the error message to stderr.
 */
static constexpr auto error_handler = [](const auto &error) {
  if constexpr (std::is_same_v<decltype(error), int>)
  {
    std::cerr << std::error_code(error, std::system_category()).message()
              << '\n'
              << std::flush;
  }
};

/**
 * @brief A simple stopped handler for asynchronous operations.
 *
 * This function is called when an error occurs in a sender chain. It prints
 * the error message to stderr.
 */
static constexpr auto stopped_handler = []() {
  std::cerr << "Stopped!" << std::endl;
};

// Forward declare the reader so that it can be used from writer().
static auto reader(async_scope &scope, const socket_dialog &client) -> void;

/**
 * @brief Asynchronously writes data to a client socket.
 * @param scope The async_scope to spawn the operation on.
 * @param client The client socket to write to.
 * @param msg The socket_message containing the data to write.
 * @param buf The buffer holding the data.
 */
static auto writer(async_scope &scope, const socket_dialog &client,
                   const socket_message &msg) -> void
{
  using namespace stdexec;
  // Create a sender that sends the message and, upon completion, either
  // continues writing if not all data was sent, or starts reading again.
  sender auto sendmsg = ::io::sendmsg(client, msg, 0) |
                        then([client, buffers = msg.buffers, &scope](auto len) {
                          // len is guaranteed >= 0 since errors are
                          // reported separately.
                          if (auto bufs = std::move(buffers); bufs += len)
                            return writer(scope, client, {.buffers = bufs});

                          if (count < NUM_ECHOES)
                            reader(scope, client);
                        }) |
                        upon_error(error_handler);

  scope.spawn(std::move(sendmsg));
}

/**
 * @brief Asynchronously reads data from a client socket.
 * @param scope The async_scope to spawn the operation on.
 * @param client The client socket to read from.
 */
static auto msg = socket_message{.buffers = read_buffer};
static auto reader(async_scope &scope, const socket_dialog &client) -> void
{
  using namespace stdexec;
  // Create a sender that receives a message and, upon completion, echoes it
  // back to the client by calling writer.
  sender auto recvmsg =
      ::io::recvmsg(client, msg, 0) | then([client, &scope](auto len) {
        // len is guaranteed >= 0 since errors are
        // reported separately.
        if (!len) // 0 indicates the client disconnected
          return;

        if (++count < NUM_ECHOES)
        {
          auto buf =
              std::span{read_buffer.data(), static_cast<std::size_t>(len)};
          writer(scope, client, {.buffers = buf});
        }
      }) |
      upon_error(error_handler);

  scope.spawn(std::move(recvmsg));
}

/**
 * @brief Creates and configures the server socket.
 * @param scope The async_scope to spawn the acceptor on.
 * @param server The server socket to configure.
 */
static auto make_server(async_scope &scope, socket_dialog server) -> void
{
  // Start the server.
  reader(scope, server);
}

/**
 * @brief Creates and configures the client socket.
 * @param scope The async_scope to spawn the acceptor on.
 * @param server The server socket to configure.
 */
static auto make_client(async_scope &scope, const socket_dialog &client) -> void
{
  // Start the client.
  writer(scope, client, {.buffers = message});
}

/**
 * @brief The main entry point of the program.
 */
static auto BM_AsyncBerkeleyEchoServer(benchmark::State &state) -> void
{
  async_scope scope;
  basic_triggers poller;

  std::array<socket_type, 2> sockets{};

  for (auto _ : state)
  {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data()))
      throw std::system_error(errno, std::system_category(), "socketpair()!");

    // Create the server.
    make_server(scope, poller.emplace(sockets[0]));

    // Create the client.
    make_client(scope, poller.emplace(sockets[1]));

    // Run the event loop until there are no more pending operations
    while (poller.wait());
    count = 0;
  }
}
BENCHMARK(BM_AsyncBerkeleyEchoServer);

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::local::stream_protocol;
namespace this_coro = boost::asio::this_coro;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
#define use_awaitable                                                          \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> echo_server(stream_protocol::socket socket)
{
  while (count < NUM_ECHOES)
  {
    std::size_t n = co_await socket.async_read_some(
        boost::asio::buffer(read_buffer), use_awaitable);
    co_await async_write(socket, boost::asio::buffer(read_buffer, n),
                         use_awaitable);
  }
}

awaitable<void> echo_client(stream_protocol::socket socket)
{
  while (count++ < NUM_ECHOES)
  {
    co_await async_write(socket,
                         boost::asio::buffer(message.data(), message.size()),
                         use_awaitable);
    co_await socket.async_read_some(boost::asio::buffer(read_buffer),
                                    use_awaitable);
  }
}

static auto BM_AsioEchoServer(benchmark::State &state) -> void
{
  boost::asio::io_context ioc(1);
  std::array<socket_type, 2> sockets;

  for (auto _ : state)
  {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data()))
      throw std::system_error(errno, std::system_category(), "socketpair()!");

    co_spawn(ioc,
             echo_server(
                 stream_protocol::socket(ioc, stream_protocol(), sockets[0])),
             detached);
    co_spawn(ioc,
             echo_client(
                 stream_protocol::socket(ioc, stream_protocol(), sockets[1])),
             detached);

    ioc.run();
    ioc.restart();
    count = 0;
  }
}
BENCHMARK(BM_AsioEchoServer);

BENCHMARK_MAIN();
// NOLINTEND
