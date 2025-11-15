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
#include "io/execution/poll_multiplexer.hpp"
#include "io/io.hpp"

#include <exec/async_scope.hpp>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

static int test_state = 0;
ssize_t recvmsg(int __fd, struct msghdr *__message, int flags)
{
  __message->msg_flags = MSG_TRUNC;
  if (test_state == 0)
  {
    errno = EWOULDBLOCK;
    test_state++;
    return -1;
  }
  return 0;
}

class MockRecvmsgTest : public ::testing::Test {
protected:
  auto SetUp() -> void { test_state = 0; }
};

TEST_F(MockRecvmsgTest, TestSyncRecvmsg)
{
  using socket_handle = ::io::socket::socket_handle;
  using message = ::io::socket::socket_message<>;

  auto sock = socket_handle{AF_UNIX, SOCK_STREAM, 0};
  auto msg = message{};
  EXPECT_EQ(::io::recvmsg(sock, msg, 0), -1);
  EXPECT_EQ(msg.flags, MSG_TRUNC);
}

TEST_F(MockRecvmsgTest, TestAsyncRecvmsg0)
{
  using namespace stdexec;
  using triggers =
      io::execution::basic_triggers<io::execution::poll_multiplexer>;
  using socket_message = ::io::socket::socket_message<>;
  using async_scope = exec::async_scope;

  auto poller = triggers();
  auto scope = async_scope();
  auto sock = poller.emplace(AF_UNIX, SOCK_STREAM, 0);
  auto msg = socket_message{};
  sender auto recvmsg =
      io::recvmsg(sock, msg, 0) | then([](auto) {}) | upon_error([](auto) {});
  scope.spawn(std::move(recvmsg));

  io::shutdown(sock, SHUT_RD);
  ASSERT_GT(poller.wait_for(50), 0);
  EXPECT_EQ(msg.flags, MSG_TRUNC);
}

TEST_F(MockRecvmsgTest, TestAsyncRecvmsg1)
{
  using namespace stdexec;
  using triggers =
      io::execution::basic_triggers<io::execution::poll_multiplexer>;

  auto poller = triggers();
  auto mtx = std::mutex();
  auto lock = std::unique_lock(mtx);
  auto cvar = std::condition_variable();
  auto started = std::atomic<bool>();

  {
    auto sock = poller.emplace(AF_UNIX, SOCK_STREAM, 0);
    auto msg = io::socket::socket_message_type();
    sender auto recvmsg = io::recvmsg(sock, msg, 0);
    auto thread = std::thread([&] {
      started = true;
      cvar.notify_all();
      sync_wait(std::move(recvmsg));
    });

    cvar.wait(lock, [&] { return started == true; });
    ::shutdown(static_cast<int>(*sock.socket), SHUT_RD);
    ASSERT_GT(poller.wait_for(50), 0);
    thread.join();
    EXPECT_EQ(msg.msg_flags, MSG_TRUNC);
  }
}
// NOLINTEND
