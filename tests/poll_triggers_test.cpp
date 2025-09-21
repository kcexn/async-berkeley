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
#include "io.hpp"

#include <exec/async_scope.hpp>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <algorithm>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

class PollTriggersTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}

  // NOLINTNEXTLINE
  basic_triggers<poll_multiplexer> triggers;
};

TEST_F(PollTriggersTest, AllocatorConstructionTest)
{
  std::allocator<std::byte> alloc;
  basic_triggers<poll_multiplexer> triggers1{alloc};
}

TEST_F(PollTriggersTest, MoveConstructorTest)
{
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

TEST_F(PollTriggersTest, MoveAssignmentTest)
{
  // NOLINTNEXTLINE
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

TEST_F(PollTriggersTest, SelfSwapTest)
{
  basic_triggers<poll_multiplexer> triggers1;
  using std::swap;
  swap(triggers1, triggers1);
  EXPECT_TRUE(&triggers1 == &triggers1);
}

TEST_F(PollTriggersTest, PushHandleTest)
{
  using socket_handle = ::io::socket::socket_handle;
  socket_handle socket{AF_INET, SOCK_STREAM, IPPROTO_TCP};
  auto sockfd = static_cast<int>(socket);

  auto dialog = triggers.push(std::move(socket));
  auto ptr = dialog.socket;
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(sockfd == *ptr);
}

TEST_F(PollTriggersTest, EmplaceHandleTest)
{
  auto dialog = triggers.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto ptr = dialog.socket;
  EXPECT_TRUE(ptr);

  auto sockfd = static_cast<int>(*ptr);

  dialog = triggers.push(std::move(*ptr));
  ptr = dialog.socket;
  EXPECT_TRUE(ptr);
  EXPECT_TRUE(*ptr == sockfd);
}

TEST_F(PollTriggersTest, PollErrorHandlingTest)
{
  handle_poll_error({EINTR, std::system_category()});
  EXPECT_THROW(handle_poll_error({EAGAIN, std::system_category()}),
               std::system_error);
}

TEST_F(PollTriggersTest, PollTest)
{
  auto list = poll_(poll_multiplexer::vector_type{}, 0);
  EXPECT_TRUE(list.empty());
}

TEST_F(PollTriggersTest, PollSetErrorTest)
{
  using socket_handle = ::io::socket::socket_handle;

  socket_handle socket{AF_INET, SOCK_STREAM, IPPROTO_TCP};
  EXPECT_NO_THROW(set_error(socket));
  EXPECT_EQ(socket.get_error().value(), 0);
  EXPECT_EQ(socket.get_error().category(), std::system_category());

  socket_handle socket2{};
  EXPECT_NO_THROW(set_error(socket2));
  EXPECT_EQ(socket2.get_error(), std::errc::bad_file_descriptor);
}

TEST_F(PollTriggersTest, PollClearEventsTest)
{
  poll_multiplexer::vector_type list{
      {.fd = 1, .events = POLLIN, .revents = POLLERR}};
  clear_events(list, list);
  EXPECT_EQ(list[0].events, 0);
}

TEST_F(PollTriggersTest, SubmitTest)
{
  using trigger = execution_trigger;
  using socket_handle = ::io::socket::socket_handle;
  using async_scope = exec::async_scope;

  async_scope scope;
  std::array<int, 2> sockets{};
  int status = ::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data());
  ASSERT_EQ(status, 0);

  std::array<char, 2> buf{};

  auto read_socket = std::make_shared<socket_handle>(sockets[0]);
  auto write_socket = std::make_shared<socket_handle>(sockets[1]);

  stdexec::sender auto read = triggers.set(read_socket, trigger::READ, [&] {
    return std::optional(
        ::read(sockets[0], buf.data(), std::min(1UL, buf.size())));
  });
  stdexec::sender auto read_future = scope.spawn_future(std::move(read));
  write(sockets[1], "a", 1);
  triggers.wait_for(0);
  EXPECT_EQ(buf[0], 'a');

  stdexec::sender auto write = triggers.set(write_socket, trigger::WRITE, [&] {
    return std::optional(::write(sockets[1], "b", 1));
  });
  stdexec::sender auto write_future = scope.spawn_future(std::move(write));
  triggers.wait_for(0);
  ::read(sockets[0], buf.data(), std::min(1UL, buf.size()));
  EXPECT_EQ(buf[0], 'b');
}

TEST_F(PollTriggersTest, WaitTest)
{
  basic_triggers<poll_multiplexer> triggers1;
  EXPECT_EQ(triggers1.wait(), 0);
}

TEST_F(PollTriggersTest, AsyncAcceptTest)
{
  using ::io::socket::make_address;
  using async_scope = exec::async_scope;

  async_scope scope;

  basic_triggers<poll_multiplexer> triggers1;
  auto dialog = triggers1.emplace(AF_INET, SOCK_STREAM, 0);

  auto address = make_address<struct sockaddr_in>();
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = INADDR_ANY;
  address->sin_port = 0;

  int status = ::io::bind(*dialog.socket, address);
  ASSERT_EQ(status, 0);

  status = ::io::listen(*dialog.socket, 1);
  ASSERT_EQ(status, 0);

  auto bound_address = make_address<struct sockaddr_in>();
  auto addr = ::io::getsockname(*dialog.socket, bound_address);
  ASSERT_EQ(bound_address, addr);

  ::io::socket::socket_handle client{AF_INET, SOCK_STREAM, 0};
  status = ::io::connect(client, bound_address);
  EXPECT_EQ(status, 0);

  stdexec::sender auto accept = ::io::accept(dialog, address);
  stdexec::sender auto accept_future = scope.spawn_future(std::move(accept));
  triggers1.wait_for(0);
  auto [result] = stdexec::sync_wait(std::move(accept_future)).value();
  auto [accept_dialog, accept_address] = std::move(result);
  EXPECT_TRUE(accept_dialog);

  auto client_address = make_address<struct sockaddr_in>();
  auto client_addr = ::io::getsockname(client, client_address);
  ASSERT_EQ(client_addr, client_address);
  EXPECT_EQ(client_address, accept_address);
}
