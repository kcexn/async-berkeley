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

#include "../src/execution/context.hpp"
#include "../src/execution/poll_multiplexer.hpp"

#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

class PollContextTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}

  basic_context<poll_multiplexer> ctx;
};

TEST_F(PollContextTest, CopyConstructorTest) {
  auto ctx2 = ctx;
  auto ptr1 = ctx.get_executor().lock();
  auto ptr2 = ctx2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
}

TEST_F(PollContextTest, CopyAssignmentTest) {
  basic_context<poll_multiplexer> ctx2;
  ctx2 = ctx;
  auto ptr1 = ctx.get_executor().lock();
  auto ptr2 = ctx2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
}

TEST_F(PollContextTest, MoveConstructorTest) {
  basic_context<poll_multiplexer> ctx1;
  auto ptr1 = ctx1.get_executor().lock();
  EXPECT_TRUE(ptr1);
  auto *addr1 = ptr1.get();

  auto ctx2{std::move(ctx1)};
  auto ptr2 = ctx2.get_executor().lock();
  EXPECT_TRUE(ptr2);
  auto *addr2 = ptr2.get();
  EXPECT_TRUE(addr1 == addr2);
}

TEST_F(PollContextTest, MoveAssignmentTest) {
  basic_context<poll_multiplexer> ctx1, ctx2;
  auto ptr1 = ctx1.get_executor().lock();
  auto ptr2 = ctx2.get_executor().lock();
  EXPECT_TRUE(ptr1 && ptr2);
  auto *addr1 = ptr1.get();
  auto *addr2 = ptr2.get();
  EXPECT_FALSE(addr1 == addr2);

  ctx2 = std::move(ctx1);
  ptr2 = ctx2.get_executor().lock();
  EXPECT_TRUE(ptr2);
  addr2 = ptr2.get();
  EXPECT_TRUE(addr2 == addr1);
}

TEST_F(PollContextTest, SelfSwapTest) {
  basic_context<poll_multiplexer> ctx1;
  using std::swap;
  swap(ctx1, ctx1);
  EXPECT_TRUE(&ctx1 == &ctx1);
}

TEST_F(PollContextTest, PushHandleTest) {
  using socket_handle = ::io::socket::socket_handle;
  socket_handle socket{AF_INET, SOCK_STREAM, IPPROTO_TCP};
  auto sockfd = static_cast<int>(socket);

  auto dialog = ctx.push(std::move(socket));
  auto ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(sockfd == *ptr);
}

TEST_F(PollContextTest, EmplaceHandleTest) {
  auto dialog = ctx.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);

  auto sockfd = static_cast<int>(*ptr);

  dialog = ctx.push(std::move(*ptr));
  ptr = dialog.socket.lock();
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(*ptr == sockfd);
}

TEST_F(PollContextTest, EraseHandleTest) {
  auto dialog = ctx.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ctx.erase(dialog);
  EXPECT_FALSE(dialog.socket.lock());
}

TEST_F(PollContextTest, SubmitTest) {
  struct test_receiver {
    using receiver_concept = stdexec::receiver_t;

    std::streamsize len_ = -1;
    int error_ = -1;

    constexpr auto set_value(std::streamsize value) noexcept -> void { len_ = value; }
    constexpr auto set_error(int error) noexcept -> void { error_ = error; }
  };

  std::array<int, 2> pipefds{};
  std::array<char, 2> buf{};
  pipe(pipefds.data());

  stdexec::sender auto send =
      ctx.submit({pipefds[0], POLLIN, 0}, [&](auto *event) {
        return ::read(pipefds[0], buf.data(), 1);
      });
  auto operation = stdexec::connect(std::move(send), test_receiver{});
  operation.start();

  write(pipefds[1], "a", 1);
  ctx.run_once();

  EXPECT_EQ(operation.receiver.len_, 1);
  EXPECT_EQ(buf[0], 'a');

  for (int file : pipefds)
    close(file);
}
