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
 * @file tcp_echo.cpp
 * @brief A simple TCP echo server using the async-berkeley library.
 *
 * This example demonstrates a basic TCP echo server that listens for incoming
 * connections and echoes back any data it receives. It showcases the use of
 * asynchronous operations for accepting connections, reading data, and writing
 * data.
 */
// NOLINTBEGIN
#include <io/io.hpp>

#include <iostream>

#include <arpa/inet.h>

// Using declarations for brevity
using namespace io;
using namespace io::socket;
using namespace io::execution;
using namespace stdexec;
using namespace exec;

// Type aliases for the specific implementations used in this example
using triggers = basic_triggers<poll_multiplexer>;
using dialog = socket_dialog<poll_multiplexer>;
using buffer = std::vector<char>;
using message = socket_message<sockaddr_in>;

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

// Forward declare the reader so that it can be used from writer().
static auto reader(async_scope &scope, const dialog &client) -> void;

/**
 * @brief Asynchronously writes data to a client socket.
 * @param scope The async_scope to spawn the operation on.
 * @param client The client socket to write to.
 * @param msg The socket_message containing the data to write.
 * @param buf The buffer holding the data.
 */
static auto writer(async_scope &scope, const dialog &client,
                   const std::shared_ptr<message> &msg,
                   const std::shared_ptr<buffer> &buf) -> void
{
  // Create a sender that sends the message and, upon completion, either
  // continues writing if not all data was sent, or starts reading again.
  sender auto send_sendmsg = sendmsg(client, *msg, 0) |
                             then([=, &scope](auto len) {
                               // len is guaranteed >= 0 since errors are
                               // reported separately.
                               if (msg->buffers += len)
                                 return writer(scope, client, msg, buf);

                               reader(scope, client);
                             }) |
                             upon_error(error_handler);

  scope.spawn(std::move(send_sendmsg));
}

/**
 * @brief Asynchronously reads data from a client socket.
 * @param scope The async_scope to spawn the operation on.
 * @param client The client socket to read from.
 */
static auto reader(async_scope &scope, const dialog &client) -> void
{
  auto msg = std::make_shared<message>();
  auto buf = std::make_shared<buffer>(1024);
  msg->buffers.push_back(*buf);

  // Create a sender that receives a message and, upon completion, echoes it
  // back to the client by calling writer.
  sender auto send_recvmsg = recvmsg(client, *msg, 0) |
                             then([=, &scope](auto len) {
                               // len is guaranteed >= 0 since errors are
                               // reported separately.
                               if (!len) // 0 indicates the client disconnected
                                 return;

                               writer(scope, client, msg, buf);
                             }) |
                             upon_error(error_handler);

  scope.spawn(std::move(send_recvmsg));
}

/**
 * @brief Asynchronously accepts incoming client connections.
 * @param scope The async_scope to spawn the acceptor on.
 * @param server The server socket to accept connections on.
 */
static auto acceptor(async_scope &scope, const dialog &server) -> void
{
  // Create a sender that accepts a new client and, upon completion,
  // starts reading from the new client and continues to accept more clients.
  sender auto send_accept = accept(server) | then([&, server](auto result) {
                              auto [client, addr] = std::move(result);
                              reader(scope, client);
                              acceptor(scope, server);
                            }) |
                            upon_error(error_handler);

  scope.spawn(std::move(send_accept));
}

/**
 * @brief Creates and configures the server socket.
 * @param scope The async_scope to spawn the acceptor on.
 * @param server The server socket to configure.
 */
static auto make_server(async_scope &scope, const dialog &server) -> void
{
  auto server_address = make_address<sockaddr_in>();
  server_address->sin_family = AF_INET;
  server_address->sin_addr.s_addr = inet_addr("127.0.0.1");
  server_address->sin_port = htons(8080);

  // Set SO_REUSEADDR to allow the server to restart quickly
  socket_option<int> reuse{1};
  if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reuse))
    throw std::system_error({errno, std::system_category()},
                            "setsockopt failed.");

  if (::io::bind(server, server_address))
    throw std::system_error({errno, std::system_category()}, "bind failed.");

  if (::io::listen(server, SOMAXCONN))
    throw std::system_error({errno, std::system_category()}, "listen failed.");

  // Start accepting connections
  acceptor(scope, server);
}

/**
 * @brief The main entry point of the program.
 */
auto main(int argc, char *argv[]) -> int
{
  async_scope scope;
  triggers trigs;

  // Create the server socket
  make_server(scope, trigs.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP));

  // Run the event loop until there are no more pending operations
  while (trigs.wait());
  return 0;
}
// NOLINTEND
