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

TEST_F(SocketMessageTest, IteratorEmptyBuffer)
{
  message_buffer<> buffers;

  // Empty buffer should have begin() == end()
  EXPECT_EQ(buffers.begin(), buffers.end());
  EXPECT_EQ(buffers.begin(), buffers.begin());
  EXPECT_EQ(buffers.end(), buffers.end());

  // Const versions
  const auto &const_buffers = buffers;
  EXPECT_EQ(const_buffers.begin(), const_buffers.end());
}

TEST_F(SocketMessageTest, IteratorNonEmptyBuffer)
{
  message_buffer<> buffers;
  std::vector<char> buf1(256, 'a');
  std::vector<char> buf2(128, 'b');
  std::vector<char> buf3(512, 'c');

  buffers.push_back(buf1);
  buffers.push_back(buf2);
  buffers.push_back(buf3);

  // Non-empty buffer should have begin() != end()
  EXPECT_NE(buffers.begin(), buffers.end());

  // Distance should match number of buffers
  EXPECT_EQ(buffers.end() - buffers.begin(), 3);
}

TEST_F(SocketMessageTest, IteratorDereference)
{
  message_buffer<> buffers;
  std::vector<char> buf1(256, 'a');
  std::vector<char> buf2(128, 'b');

  buffers.push_back(buf1);
  buffers.push_back(buf2);

  auto it = buffers.begin();

  // Dereference should return std::span<std::byte>
  auto span1 = *it;
  EXPECT_EQ(span1.size(), 256);

  ++it;
  auto span2 = *it;
  EXPECT_EQ(span2.size(), 128);
}

TEST_F(SocketMessageTest, IteratorConstVersion)
{
  message_buffer<> buffers;
  std::vector<char> buf1(256, 'a');
  std::vector<char> buf2(128, 'b');

  buffers.push_back(buf1);
  buffers.push_back(buf2);

  const auto &const_buffers = buffers;

  // Const iterators should work
  auto it = const_buffers.begin();
  EXPECT_NE(it, const_buffers.end());

  auto span1 = *it;
  EXPECT_EQ(span1.size(), 256);

  ++it;
  auto span2 = *it;
  EXPECT_EQ(span2.size(), 128);

  ++it;
  EXPECT_EQ(it, const_buffers.end());
}

TEST_F(SocketMessageTest, IteratorIncrement)
{
  message_buffer<> buffers;
  std::vector<char> buf1(100);
  std::vector<char> buf2(200);
  std::vector<char> buf3(300);

  buffers.push_back(buf1);
  buffers.push_back(buf2);
  buffers.push_back(buf3);

  auto it = buffers.begin();

  // Pre-increment
  EXPECT_EQ((*it).size(), 100);
  auto &ref = ++it;
  EXPECT_EQ(&ref, &it); // Should return reference to self
  EXPECT_EQ((*it).size(), 200);

  // Post-increment
  auto old = it++;
  EXPECT_EQ((*old).size(), 200);
  EXPECT_EQ((*it).size(), 300);

  ++it;
  EXPECT_EQ(it, buffers.end());
}

TEST_F(SocketMessageTest, IteratorRandomAccess)
{
  message_buffer<> buffers;
  std::vector<char> buf1(100);
  std::vector<char> buf2(200);
  std::vector<char> buf3(300);

  buffers.push_back(buf1);
  buffers.push_back(buf2);
  buffers.push_back(buf3);

  auto it = buffers.begin();

  // Subscript operator
  EXPECT_EQ(it[0].size(), 100);
  EXPECT_EQ(it[1].size(), 200);
  EXPECT_EQ(it[2].size(), 300);

  // Addition
  auto it2 = it + 2;
  EXPECT_EQ((*it2).size(), 300);

  // Subtraction
  auto it3 = it2 - 1;
  EXPECT_EQ((*it3).size(), 200);

  // Compound assignment
  it += 2;
  EXPECT_EQ((*it).size(), 300);

  it -= 1;
  EXPECT_EQ((*it).size(), 200);
}

TEST_F(SocketMessageTest, IteratorComparison)
{
  message_buffer<> buffers;
  std::vector<char> buf1(100);
  std::vector<char> buf2(200);

  buffers.push_back(buf1);
  buffers.push_back(buf2);

  auto it1 = buffers.begin();
  auto it2 = buffers.begin();
  auto it3 = buffers.begin() + 1;

  // Equality
  EXPECT_EQ(it1, it2);
  EXPECT_NE(it1, it3);

  // Ordering
  EXPECT_LT(it1, it3);
  EXPECT_LE(it1, it3);
  EXPECT_LE(it1, it2);
  EXPECT_GT(it3, it1);
  EXPECT_GE(it3, it1);
  EXPECT_GE(it1, it2);
}

TEST_F(SocketMessageTest, IteratorRangeBasedForLoop)
{
  message_buffer<> buffers;
  std::vector<char> buf1(100);
  std::vector<char> buf2(200);
  std::vector<char> buf3(300);

  buffers.push_back(buf1);
  buffers.push_back(buf2);
  buffers.push_back(buf3);

  std::vector<std::size_t> sizes;
  for (auto span : buffers)
  {
    sizes.push_back(span.size());
  }

  ASSERT_EQ(sizes.size(), 3);
  EXPECT_EQ(sizes[0], 100);
  EXPECT_EQ(sizes[1], 200);
  EXPECT_EQ(sizes[2], 300);
}

TEST_F(SocketMessageTest, IteratorConstRangeBasedForLoop)
{
  message_buffer<> buffers;
  std::vector<char> buf1(100);
  std::vector<char> buf2(200);

  buffers.push_back(buf1);
  buffers.push_back(buf2);

  const auto &const_buffers = buffers;

  std::vector<std::size_t> sizes;
  for (auto span : const_buffers)
  {
    sizes.push_back(span.size());
  }

  ASSERT_EQ(sizes.size(), 2);
  EXPECT_EQ(sizes[0], 100);
  EXPECT_EQ(sizes[1], 200);
}
// NOLINTEND
