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
#include "../src/io.hpp"

#include <gtest/gtest.h>

#include <sys/socket.h>

using namespace io::socket;

class SocketMessageTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketMessageTest, SendRecvMsgTest) {
  socket_message message{};
  std::array<int, 2> pair{};
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, pair.data()), 0);
  socket_handle sender{pair[0]};
  socket_handle receiver{pair[1]};

  // NOLINTNEXTLINE
  std::array<char, 14> msg{"Hello, world!"};
  message.buffers.emplace_back(msg.data(), msg.size());

  auto len = ::io::sendmsg(sender, message, 0);
  EXPECT_EQ(len, 14);
  msg = {};
  EXPECT_NE(std::strncmp(msg.data(), "Hello, world!", 14), 0);

  auto addr = make_address<sockaddr_un>();
  auto result = ::io::getsockname(sender, addr);
  ASSERT_NE(result.data(), nullptr);
  ASSERT_NE(addr, result);
  addr = result;
  ASSERT_EQ(addr, result);
  message.address = addr;

  len = ::io::recvmsg(receiver, message, 0);
  EXPECT_EQ(len, 14);
  EXPECT_EQ(std::strncmp(msg.data(), "Hello, world!", 14), 0);
}
