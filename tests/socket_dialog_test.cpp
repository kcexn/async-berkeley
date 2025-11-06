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
// NOLINTBEGIN
#include "io/io.hpp"

#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::socket;
using namespace io::execution;
using namespace io::socket::detail;

class SocketDialogTest : public ::testing::TestWithParam<bool> {
protected:
  void SetUp() override { is_lazy_ = GetParam(); }
  void TearDown() override {}

  bool is_lazy_ = false;
  basic_triggers<poll_multiplexer> triggers;
};

TEST_P(SocketDialogTest, ConnectAcceptOperation)
{
  using async_scope = exec::async_scope;

  async_scope scope;

  if (is_lazy_)
  {
    // Set the fairness counter to max value to force lazy evaluation.
    fairness::counter() = -1;
  }
  auto accept_dialog = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto connect_dialog = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto address = make_address<sockaddr_in>();
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = INADDR_ANY;
  address->sin_port = 0;

  int status = ::io::bind(accept_dialog, address);
  ASSERT_EQ(status, 0);

  status = ::io::listen(accept_dialog, 1);
  ASSERT_EQ(status, 0);

  auto bound_address = make_address<sockaddr_in>();
  auto addr = ::io::getsockname(accept_dialog, bound_address);
  ASSERT_EQ(bound_address, addr);

  auto client_addr = make_address<sockaddr_in>();
  stdexec::sender auto connect = ::io::connect(connect_dialog, bound_address);
  stdexec::sender auto accept = ::io::accept(accept_dialog, client_addr);
  stdexec::sender auto connect_accept_future = scope.spawn_future(
      stdexec::when_all(std::move(accept), std::move(connect)));

  while (triggers.wait_for(0));

  auto [accept_result, connect_result] =
      stdexec::sync_wait(std::move(connect_accept_future)).value();

  EXPECT_EQ(connect_result, 0);
  auto [dialog, handle_addr] = std::move(accept_result);
  EXPECT_TRUE(dialog);
}

TEST_P(SocketDialogTest, SendmsgRecvmsgOperation)
{
  using async_scope = exec::async_scope;

  async_scope scope;
  const char *message = "Hello, World!";
  void *send_buf = reinterpret_cast<void *>(const_cast<char *>(message));
  std::array<char, 14> recv_buf{};
  socket_message send_msg;
  send_msg.buffers.emplace_back(send_buf, ::strnlen(message, 14));
  socket_message recv_msg;
  recv_msg.buffers.emplace_back(recv_buf.data(), recv_buf.size());

  std::array<native_socket_type, 2> pair{};
  int status = ::socketpair(AF_UNIX, SOCK_STREAM, 0, pair.data());
  ASSERT_EQ(status, 0);

  auto send_dialog = triggers.emplace(pair[0]);
  auto recv_dialog = triggers.emplace(pair[1]);

  if (is_lazy_)
  {
    // Set the fairness counter to max value to force lazy evaluation.
    fairness::counter() = -1;
  }

  stdexec::sender auto send_sender = ::io::sendmsg(send_dialog, send_msg, 0);
  stdexec::sender auto send_future = scope.spawn_future(std::move(send_sender));
  while (triggers.wait_for(0));

  if (is_lazy_)
  {
    // Set the fairness counter to max value to force lazy evaluation.
    fairness::counter() = -1;
  }

  stdexec::sender auto recv_sender = ::io::recvmsg(recv_dialog, recv_msg, 0);
  stdexec::sender auto recv_future = scope.spawn_future(std::move(recv_sender));
  while (triggers.wait_for(0));

  auto [send_len, recv_len] =
      stdexec::sync_wait(
          stdexec::when_all(std::move(send_future), std::move(recv_future)))
          .value();

  EXPECT_EQ(send_len, recv_len);
  EXPECT_EQ(::strncmp(message, recv_buf.data(), 14), 0);
}

INSTANTIATE_TEST_SUITE_P(SocketDialogTests, SocketDialogTest, ::testing::Bool(),
                         [](const auto &info) {
                           return info.param ? "Lazy" : "Normal";
                         });

class SocketDialogHelperTest : public ::testing::Test {};

TEST_F(SocketDialogHelperTest, getExecutorTest)
{
  using socket_dialog = socket_dialog<poll_multiplexer>;
  auto dialog = socket_dialog{};
  EXPECT_THROW(get_executor(dialog), std::invalid_argument);
}

TEST_F(SocketDialogHelperTest, HandleConnectErrorTest)
{
  using triggers_type = basic_triggers<poll_multiplexer>;

  auto poller = triggers_type();
  auto dialog = poller.emplace(AF_UNIX, SOCK_STREAM, 0);
  errno = ENOMEM;

  handle_connect_error(dialog);
  EXPECT_EQ(dialog.socket->get_error(), std::errc::not_enough_memory);
}

TEST_F(SocketDialogHelperTest, SetErrorIfNotBlocked)
{
  using socket_handle = ::io::socket::socket_handle;

  auto sock = socket_handle{AF_UNIX, SOCK_STREAM, 0};
  EXPECT_FALSE(set_error_if_not_blocked(sock, EWOULDBLOCK));
  EXPECT_FALSE(set_error_if_not_blocked(sock, EAGAIN));
  EXPECT_TRUE(set_error_if_not_blocked(sock, ENOMEM));
}

TEST_F(SocketDialogHelperTest, SetSockOptTest)
{
  using triggers_type = basic_triggers<poll_multiplexer>;

  auto poller = triggers_type{};
  auto dialog = poller.emplace(AF_UNIX, SOCK_STREAM, 0);

  auto reuse = socket_option<int>{1};
  EXPECT_EQ(::io::setsockopt(dialog, SOL_SOCKET, SO_REUSEADDR, reuse), 0);
}

TEST_F(SocketDialogHelperTest, ShutdownTest)
{
  using triggers_type = basic_triggers<poll_multiplexer>;
  auto poller = triggers_type{};
  auto dialog = poller.emplace(AF_UNIX, SOCK_STREAM, 0);

  EXPECT_EQ(::io::shutdown(dialog, SHUT_RD), 0);
}

class SocketDialogComparisonTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    dialog1 = triggers.emplace(AF_INET, SOCK_STREAM, 0);
    dialog2 = triggers.emplace(AF_INET, SOCK_STREAM, 0);
  }

  basic_triggers<poll_multiplexer> triggers;
  socket_dialog<poll_multiplexer> dialog1;
  socket_dialog<poll_multiplexer> dialog2;
};

TEST_F(SocketDialogComparisonTest, DialogToDialogComparison)

{
  EXPECT_EQ(dialog1, dialog1);
  EXPECT_NE(dialog1, dialog2);
  EXPECT_LT(dialog1, dialog2);
}

TEST_F(SocketDialogComparisonTest, DialogToHandleComparison)
{
  auto &handle = *dialog1.socket;
  EXPECT_EQ(dialog1, handle);
  EXPECT_NE(dialog2, handle);
}

TEST_F(SocketDialogComparisonTest, DialogToNativeComparison)
{
  auto native_handle = static_cast<native_socket_type>(*dialog1.socket);
  EXPECT_EQ(dialog1, native_handle);
  EXPECT_NE(dialog2, native_handle);
}

TEST_F(SocketDialogComparisonTest, InvalidDialogThrowsException)
{
  auto invalid_dialog = socket_dialog<poll_multiplexer>{};
  EXPECT_THROW((void)(invalid_dialog <=> dialog1), std::invalid_argument);
  EXPECT_THROW((void)(dialog1 <=> invalid_dialog), std::invalid_argument);
}

// NOLINTEND
