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
#include "../src/io/execution/poll_multiplexer.hpp"
#include "../src/io/execution/triggers.hpp"

#include <exec/async_scope.hpp>
#include <exec/static_thread_pool.hpp>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

class PollContextTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}

  basic_triggers<poll_multiplexer> triggers;
};

TEST_F(PollContextTest, CopyConstructorTest) {
  auto triggers2 = triggers;
  auto ptr1 = triggers.get_executor().lock();
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
}

TEST_F(PollContextTest, CopyAssignmentTest) {
  basic_triggers<poll_multiplexer> triggers2;
  triggers2 = triggers;
  auto ptr1 = triggers.get_executor().lock();
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
}

TEST_F(PollContextTest, MoveConstructorTest) {
  basic_triggers<poll_multiplexer> triggers1;
  auto ptr1 = triggers1.get_executor().lock();
  EXPECT_TRUE(ptr1);
  auto *addr1 = ptr1.get();

  auto triggers2{std::move(triggers1)};
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr2);
  auto *addr2 = ptr2.get();
  EXPECT_TRUE(addr1 == addr2);
}

TEST_F(PollContextTest, MoveAssignmentTest) {
  basic_triggers<poll_multiplexer> triggers1, triggers2;
  auto ptr1 = triggers1.get_executor().lock();
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr1 && ptr2);
  auto *addr1 = ptr1.get();
  auto *addr2 = ptr2.get();
  EXPECT_FALSE(addr1 == addr2);

  triggers2 = std::move(triggers1);
  ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr2);
  addr2 = ptr2.get();
  EXPECT_TRUE(addr2 == addr1);
}

TEST_F(PollContextTest, SelfSwapTest) {
  basic_triggers<poll_multiplexer> triggers1;
  using std::swap;
  swap(triggers1, triggers1);
  EXPECT_TRUE(&triggers1 == &triggers1);
}

TEST_F(PollContextTest, PushHandleTest) {
  using socket_handle = ::io::socket::socket_handle;
  socket_handle socket{AF_INET, SOCK_STREAM, IPPROTO_TCP};
  auto sockfd = static_cast<int>(socket);

  auto dialog = triggers.push(std::move(socket));
  auto ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(sockfd == *ptr);
}

TEST_F(PollContextTest, EmplaceHandleTest) {
  auto dialog = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);

  auto sockfd = static_cast<int>(*ptr);

  dialog = triggers.push(std::move(*ptr));
  ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(*ptr == sockfd);
}

TEST_F(PollContextTest, EraseHandleTest) {
  auto dialog = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  triggers.erase(dialog);
  EXPECT_FALSE(dialog.socket.lock());
}

TEST_F(PollContextTest, PollErrorHandlingTest) {
  handle_poll_error(EINTR);
  EXPECT_THROW(handle_poll_error(EAGAIN), std::system_error);
}

TEST_F(PollContextTest, SubmitTest) {
  std::array<int, 2> pipefds{};
  std::array<char, 3> buf{};
  pipe(pipefds.data());

  stdexec::sender auto read = triggers.set({pipefds[0], POLLIN, 0}, [&] {
    return ::read(pipefds[0], buf.data(), 1);
  });
  write(pipefds[1], "a", 1);
  triggers.wait_for(0);
  EXPECT_EQ(buf[0], 'a');

  stdexec::sender auto write = triggers.set(
      {pipefds[1], POLLOUT, 0}, [&] { return ::write(pipefds[1], "b", 1); });
  triggers.wait_for(0);
  ::read(pipefds[0], buf.data(), 1);
  EXPECT_EQ(buf[0], 'b');

  for (int file : pipefds)
    close(file);
}
