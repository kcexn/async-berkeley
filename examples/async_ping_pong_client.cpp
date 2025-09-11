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
 * @file async_ping_pong_client.cpp
 * @brief Asynchronous ping/pong client example using the iosched library.
 *
 * This client connects to a server, sends "ping" messages, and waits for
 * "pong" responses using the asynchronous API.
 */

#include <exec/async_scope.hpp>
#include <io.hpp>
#include <stdexec/execution.hpp>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

#include <arpa/inet.h>
#include <netinet/in.h>

using namespace io;
using namespace io::socket;
using namespace io::execution;
using namespace stdexec;
using namespace exec;

template <typename T> class AsyncPingPongClient {
public:
  AsyncPingPongClient(socket_address<T> server, int ping_count)
      : server_(std::move(server)), ping_count_{ping_count}, triggers_() {
    // NOLINTNEXTLINE
    static std::array<char, 6> pong_buf{};
    pong_msg.buffers.emplace_back(pong_buf.data(), pong_buf.size());
  }

  void run() {
    // Create client socket dialog using triggers
    auto client = triggers_.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Connect asynchronously
    auto connect_sender = io::connect(client, server_);

    // Create continuation for connection completion
    auto continuation = then(std::move(connect_sender),
                             [this, client](const auto &connect_result) {
                               // Start ping/pong sequence
                               start_ping_pong(client, 0);
                             });

    // start ping_pong.
    scope_.spawn(upon_error(std::move(continuation), [](auto &&error) {}));

    // Wait for completion
    while (pings_sent_ < ping_count_ || pongs_received_ < ping_count_) {
      triggers_.wait();
    }
  }

private:
  void start_ping_pong(const socket_dialog<poll_multiplexer> &client_dialog,
                       int sequence) {
    if (sequence < ping_count_)
      send_ping(client_dialog, sequence);
  }

  void send_ping(const socket_dialog<poll_multiplexer> &client, int sequence) {
    // Create ping message.
    const char *ping_message = "ping!\n";
    socket_message message{};
    // NOLINTNEXTLINE
    message.buffers.emplace_back(const_cast<char *>(ping_message),
                                 ::strlen(ping_message));

    // Send ping asynchronously
    auto send = io::sendmsg(client, message, 0);

    // Create continuation for send completion
    auto continuation = then(
        std::move(send), [this, client, sequence](const auto &send_result) {
          std::cout << "Sent: " << ++pings_sent_ << " pings.\n" << std::flush;

          // Wait for pong response
          wait_for_pong(client, sequence);
        });

    // Start the send operation
    scope_.spawn(upon_error(std::move(continuation), [](auto &&error) {}));
  }

  void wait_for_pong(const socket_dialog<poll_multiplexer> &client,
                     int sequence) {
    // Receive message asynchronously
    auto recv = io::recvmsg(client, pong_msg, 0);

    // Create continuation for handling received data
    auto continuation =
        then(std::move(recv),
             [this, client, sequence,
              buf = pong_msg.buffers.front()](std::streamsize bytes_received) {
               std::string message(static_cast<char *>(buf.iov_base),
                                   bytes_received);
               // Check if it's a pong response
               if (message.find("pong") != std::string::npos) {
                 pongs_received_++;

                 // Send next ping after a short delay
                 schedule_next_ping(client, sequence + 1);
               }
             });

    // Start the receive operation
    scope_.spawn(upon_error(std::move(continuation), [](auto &&error) {}));
  }

  void schedule_next_ping(const socket_dialog<poll_multiplexer> &client,
                          int next_sequence) {
    // Add a small delay between pings to make the demo more visible
    // NOLINTNEXTLINE
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Continue with next ping if we haven't reached the limit
    if (next_sequence < ping_count_)
      start_ping_pong(client, next_sequence);
  }

  socket_address<T> server_;
  int ping_count_{};
  std::atomic<int> pings_sent_;
  std::atomic<int> pongs_received_;
  basic_triggers<poll_multiplexer> triggers_;
  async_scope scope_;
  socket_message pong_msg{};
};

auto main(int argc, char *argv[]) -> int {

  auto server = make_address<sockaddr_in>();
  server->sin_family = AF_INET;
  server->sin_addr.s_addr = inet_addr("127.0.0.1");
  // NOLINTNEXTLINE
  server->sin_port = htons(8080);
  // NOLINTNEXTLINE
  int ping_count = 5;

  AsyncPingPongClient client(server, ping_count);
  client.run();

  return 0;
}
