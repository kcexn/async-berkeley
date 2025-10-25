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

#include <sys/socket.h>

using namespace io::socket;

class SocketMessageTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketMessageTest, CustomAllocatorConstruction)
{
  auto alloc = std::allocator<native_buffer_type>();
  auto msg = message_buffer(alloc);
}

TEST_F(SocketMessageTest, SendRecvMsgTest)
{
  socket_message message;
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

  auto addr = socket_address<sockaddr_un>();
  message.address = ::io::getsockname(sender, addr);

  len = ::io::recvmsg(receiver, message, 0);
  EXPECT_EQ(len, 14);
  EXPECT_EQ(std::strncmp(msg.data(), "Hello, world!", 14), 0);
}

TEST_F(SocketMessageTest, CompoundAdditionTest)
{
  std::vector<char> buf1(256);
  std::vector<char> buf2(256);
  socket_message message{};

  auto &buffers = message.buffers;
  buffers.push_back(buf1);
  buffers.push_back(buf2);
  ASSERT_EQ(buffers.size(), 2);

  buffers += 256;
  EXPECT_EQ(buffers.size(), 1);

  buffers.push_back(buf1);
  EXPECT_EQ(buffers.size(), 2);

  buffers += 128;
  EXPECT_EQ(buffers.size(), 2);

  buffers += 128;
  EXPECT_EQ(buffers.size(), 1);

  buffers += 512;
  EXPECT_TRUE(buffers.empty());

  buffers.emplace_back(nullptr, 0);
  EXPECT_TRUE(buffers.empty());

  buffers += 512;
  EXPECT_FALSE(buffers);
}
// NOLINTEND
