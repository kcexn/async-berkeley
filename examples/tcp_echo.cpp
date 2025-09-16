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
// NOLINTBEGIN
#include <io.hpp>

#include <iostream>

#include <arpa/inet.h>

using namespace io;
using namespace io::socket;
using namespace io::execution;
using namespace stdexec;
using namespace exec;

using poll_triggers = basic_triggers<poll_multiplexer>;
using poll_dialog = socket_dialog<poll_multiplexer>;

static constexpr auto error_handler = [](const auto &error) {
  if constexpr (std::is_same_v<decltype(error), int>)
  {
    std::cerr << std::error_code(error, std::system_category()).message()
              << '\n'
              << std::flush;
  }
};

auto reader(async_scope &scope, const poll_dialog &client) -> void;
static auto writer(async_scope &scope, const poll_dialog &client,
                   const std::vector<char> &buf) -> void
{
  auto writebuf = std::make_shared<std::vector<char>>(buf.cbegin(), buf.cend());
  socket_message msg;
  msg.buffers.emplace_back(writebuf->data(), writebuf->size());
  sender auto send_sendmsg =
      sendmsg(client, msg, 0) | then([&, client, buf = writebuf](auto len) {
        if (static_cast<std::size_t>(len) < buf->size())
        {
          buf->erase(buf->begin(), buf->begin() + len);
          writer(scope, client, *buf);
          return;
        }
        reader(scope, client);
      }) |
      upon_error(error_handler);

  scope.spawn(std::move(send_sendmsg));
}

auto reader(async_scope &scope, const poll_dialog &client) -> void
{
  auto msg = std::make_shared<socket_message>();
  auto readbuf = std::make_shared<std::vector<char>>(1024);
  msg->buffers.emplace_back(readbuf->data(), readbuf->size());
  sender auto send_recvmsg =
      recvmsg(client, *msg, 0) |
      then([&, client, msg, buf = readbuf](auto len) {
        if (len > 0)
        {
          buf->erase(buf->begin() + len, buf->end());
          writer(scope, client, *buf);
        }
      }) |
      upon_error(error_handler);

  scope.spawn(std::move(send_recvmsg));
}

static auto acceptor(async_scope &scope, const poll_dialog &server) -> void
{
  sender auto send_accept =
      accept(server) | then([&](auto result) {
        auto [client, addr] = std::move(result);
        reader(scope, client);
        acceptor(scope, server);
      }) |
      upon_error(error_handler);

  scope.spawn(std::move(send_accept));
}

static auto make_server(async_scope &scope, const poll_dialog &server) -> void
{
  auto server_address = make_address<sockaddr_in>();
  server_address->sin_family = AF_INET;
  server_address->sin_addr.s_addr = inet_addr("127.0.0.1");
  server_address->sin_port = htons(8080);

  socket_option<int> reuse{1};
  if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, reuse))
    throw std::system_error({errno, std::system_category()},
                            "setsockopt failed.");

  if (bind(server, server_address))
    throw std::system_error({errno, std::system_category()}, "bind failed.");

  if (listen(server, SOMAXCONN))
    throw std::system_error({errno, std::system_category()}, "listen failed.");

  acceptor(scope, server);
}

auto main(int argc, char *argv[]) -> int
{
  try
  {
    async_scope scope;
    poll_triggers triggers;

    auto server = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    make_server(scope, server);

    while (triggers.wait());
    return 0;
  }
  catch (...)
  {
    return 1;
  }
}
// NOLINTEND
