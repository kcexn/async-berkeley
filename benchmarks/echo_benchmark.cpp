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
 * @file echo_benchmark.cpp
 * @brief A simple benchmark comparing AsyncBerkeley against ASIO.
 *
 * Initial benchmarking suggests that AsyncBerkeley could have nearly
 * 50% more throughput than Asio.
 *
 * 2025-09-24T14:25:00+10:00
 * Run on (12 X 2496.01 MHz CPU s)
 * CPU Caches:
 *  L1 Data 48 KiB (x6)
 *  L1 Instruction 32 KiB (x6)
 *  L2 Unified 1280 KiB (x6)
 *  L3 Unified 12288 KiB (x1)
 * Load Average: 0.72, 0.79, 0.72
 * -----------------------------------------------------------------------
 * Benchmark                            Time             CPU   Iterations
 * -----------------------------------------------------------------------
 * AsyncBerkeley/64/100/100          20.4 ms         18.7 ms           36
 * AsyncBerkeley/64/100000/100      21619 ms        19686 ms            1
 * Asio/64/100/100                   30.4 ms         27.5 ms           23
 * Asio/64/100000/100               29263 ms        26638 ms            1
 *
 * A follow-up benchmark conducted without '-march=native' and
 * '-mtune=native' compiler optimization flags indicates only a 20%
 * increase in throughput. This suggests that AsyncBerkeley is much
 * easier to optimize than Asio.
 *
 * -----------------------------------------------------------------------
 * Benchmark                            Time             CPU   Iterations
 * -----------------------------------------------------------------------
 * AsyncBerkeley/64/100/100          27.2 ms         24.8 ms           27
 * AsyncBerkeley/64/100000/100      24147 ms        22010 ms            1
 * Asio/64/100/100                   33.3 ms         30.2 ms           22
 * Asio/64/100000/100               29110 ms        26544 ms            1
 */
// NOLINTBEGIN
#include <benchmark/benchmark.h>
#include <io/io.hpp>

#include <boost/asio.hpp>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>

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

/**
 * @class BaseEchoFixture
 * @brief Base fixture for echo benchmarks, providing common configuration.
 */
class BaseEchoFixture : public benchmark::Fixture {
public:
  void SetUp(benchmark::State &state) override
  {
    bufsize = state.range(0);
    iterations = state.range(1);
    connections = state.range(2);
    message.resize(bufsize, 'x');
  }
  void TearDown(benchmark::State &state) override {}

  std::size_t bufsize;
  std::size_t iterations;
  std::size_t connections;
  std::string message;
};

/**
 * @class AsyncBerkeleyEchoFixture
 * @brief Fixture for benchmarking the async-berkeley echo implementation.
 */
class AsyncBerkeleyEchoFixture : public BaseEchoFixture {
public:
  /**
   * @struct session
   * @brief Manages an echo session, handling reading and writing of data.
   */
  struct session {

    /** @brief Simple error handler that prints to stderr. */
    static constexpr auto error_handler = [](const auto &error) {
      std::cerr << "Error!" << std::endl;
    };
    /** @brief Simple stopped handler that prints to stderr. */
    static constexpr auto stopped_handler = []() {
      std::cerr << "Stopped!" << std::endl;
    };

    /** @brief Buffer for reading data from the socket. */
    std::vector<std::byte> read_buffer;
    /** @brief Message object for socket operations. */
    socket_message msg;
    /** @brief Counter for the number of iterations. */
    std::size_t count{0};

    explicit session(std::size_t bufsize)
        : read_buffer(bufsize), msg{.buffers = read_buffer}
    {}

    /**
     * @brief Asynchronously writes data to the client and schedules the next
     * read.
     * @param scope The async_scope to spawn the operation on.
     * @param client The socket_dialog to write to.
     * @param msg The message to be written.
     */
    auto writer(async_scope &scope, const socket_dialog &client,
                const socket_message &msg, std::size_t iterations) -> void
    {
      using namespace stdexec;
      auto sendmsg =
          ::io::sendmsg(client, msg, 0) |
          then([this, client, buffers = msg.buffers, &scope,
                iterations](auto len) {
            if (auto bufs = std::move(buffers); bufs += len)
              return writer(scope, client, {.buffers = bufs}, iterations);

            if (count < iterations)
              reader(scope, client, iterations);
          }) |
          upon_error(error_handler) | upon_stopped(stopped_handler);

      scope.spawn(std::move(sendmsg));
    }

    /**
     * @brief Asynchronously reads data from the client and schedules a write.
     * @param scope The async_scope to spawn the operation on.
     * @param client The socket_dialog to read from.
     */
    auto reader(async_scope &scope, const socket_dialog &client,
                std::size_t iterations) -> void
    {
      using namespace stdexec;
      sender auto recvmsg =
          ::io::recvmsg(client, msg, 0) |
          then([this, client, &scope, iterations](auto len) {
            if (len && ++count < iterations)
            {
              auto buf =
                  std::span{read_buffer.data(), static_cast<std::size_t>(len)};
              writer(scope, client, {.buffers = buf}, iterations);
            }
          }) |
          upon_error(error_handler) | upon_stopped(stopped_handler);

      scope.spawn(std::move(recvmsg));
    }
  };
};

/**
 * @brief Benchmark for the async-berkeley echo implementation.
 *
 * This benchmark measures the performance of the echo implementation by
 * creating a number of socket pairs and echoing data between them.
 */
BENCHMARK_DEFINE_F(AsyncBerkeleyEchoFixture, EchoTest)
(benchmark::State &state)
{
  async_scope scope;
  basic_triggers poller;

  std::vector<socket_type> sockets(2 * connections);
  std::vector<session> sessions;
  sessions.reserve(connections);

  for (auto _ : state)
  {
    for (std::size_t i = 0; i < sockets.size(); i += 2)
    {
      if (::socketpair(AF_UNIX, SOCK_STREAM, 0, &sockets[i]))
        throw std::system_error(errno, std::system_category(), "socketpair()!");
      auto &echo = sessions.emplace_back(bufsize);
      echo.reader(scope, poller.emplace(sockets[i]), iterations);
      echo.writer(scope, poller.emplace(sockets[i + 1]), {.buffers = message},
                  iterations);
    }
    while (poller.wait());
  }
}
BENCHMARK_REGISTER_F(AsyncBerkeleyEchoFixture, EchoTest)
    ->Args({64, 100, 100})
    ->Args({64, 100, 1000})
    ->Args({64, 1000, 100})
    ->Args({64, 100000, 100})
    ->Unit(benchmark::kMillisecond);


using namespace boost::asio;
using local::stream_protocol;

/** @brief A benchmark fixture for ASIO echo server. */
class AsioEchoFixture : public BaseEchoFixture {
public:
  /** @brief Represents a single echo session. */
  struct echo_session {
    /** @brief The socket for the session. */
    stream_protocol::socket socket;
    /** @brief The data buffer. */
    std::vector<char> data;
    /** @brief The number of messages sent. */
    std::size_t count = 0;

    /**
     * @brief Construct a new echo session object.
     * @param ctx The io_context.
     * @param bufsize The buffer size.
     */
    explicit echo_session(io_context &ctx, std::size_t bufsize)
        : socket(ctx), data(bufsize)
    {}

    friend void do_read(std::unique_ptr<echo_session> self,
                        std::size_t iterations);
    friend void do_write(std::unique_ptr<echo_session> self, std::size_t n,
                         std::size_t iterations);
  };
};

/**
 * @brief Asynchronously reads from the socket.
 * @param self The echo session.
 * @param iterations The number of iterations.
 */
void do_read(std::unique_ptr<AsioEchoFixture::echo_session> self,
             std::size_t iterations)
{
  auto &socket = self->socket;
  auto buffer = boost::asio::buffer(self->data);
  socket.async_read_some(
      buffer, [self = std::move(self), iterations](auto error, auto n) mutable {
        if (!error)
          do_write(std::move(self), n, iterations);
      });
}

/**
 * @brief Asynchronously writes to the socket.
 * @param self The echo session.
 * @param n The number of bytes to write.
 * @param iterations The number of iterations.
 */
void do_write(std::unique_ptr<AsioEchoFixture::echo_session> self,
              std::size_t n, std::size_t iterations)
{
  auto &socket = self->socket;
  auto buffer = boost::asio::buffer(self->data, n);
  boost::asio::async_write(
      socket, buffer,
      [self = std::move(self), iterations](auto error, auto) mutable {
        if (!error && self->count++ < iterations)
          do_read(std::move(self), iterations);
      });
}

/**
 * @brief The benchmark for the ASIO echo test.
 *
 * This benchmark is based off of the benchmark in Kohlhoff's clearpool.io blog
 * post.
 *
 * [What's New in Asio - Performance]
 * (https://clearpool.io/pulse/posts/2020/Jul/13/whats-new-in-asio-performance/)
 *
 * July 13, 2020.
 */
BENCHMARK_DEFINE_F(AsioEchoFixture, EchoTest)(benchmark::State &state)
{
  io_context ctx(1);
  for (auto _ : state)
  {
    for (std::size_t i = 0; i < connections; ++i)
    {
      auto session1 = std::make_unique<echo_session>(ctx, bufsize);
      auto session2 = std::make_unique<echo_session>(ctx, bufsize);
      connect_pair(session1->socket, session2->socket);
      do_read(std::move(session1), iterations);
      do_write(std::move(session2), bufsize, iterations);
    }
    ctx.run();
    ctx.reset();
  }
}
BENCHMARK_REGISTER_F(AsioEchoFixture, EchoTest)
    ->Args({64, 100, 100})
    ->Args({64, 100, 1000})
    ->Args({64, 1000, 100})
    ->Args({64, 100000, 100})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
// NOLINTEND
