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
#include "io/execution/poll_multiplexer.hpp"
#include "io/execution/triggers.hpp"
#include "io/socket/socket.hpp"

#include <exec/async_scope.hpp>
#include <exec/static_thread_pool.hpp>
#include <gtest/gtest.h>
#include <io/detail/customization.hpp>
#include <stdexec/execution.hpp>

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

TEST_F(PollTriggersTest, CopyConstructorTest)
{
  auto triggers2 = triggers;
  auto ptr1 = triggers.get_executor().lock();
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
}

TEST_F(PollTriggersTest, CopyAssignmentTest)
{
  basic_triggers<poll_multiplexer> triggers2;
  triggers2 = triggers;
  auto ptr1 = triggers.get_executor().lock();
  auto ptr2 = triggers2.get_executor().lock();
  EXPECT_TRUE(ptr1.get() == ptr2.get());
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
  handle_poll_error(EINTR);
  EXPECT_THROW(handle_poll_error(EAGAIN), std::system_error);
}

TEST_F(PollTriggersTest, PollTest)
{
  auto list = poll_({}, 0);
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
  EXPECT_EQ(socket2.get_error().value(), EBADF);
}

TEST_F(PollTriggersTest, PollPrepareHandlesTest)
{
  using socket_handle = ::io::socket::socket_handle;

  poll_multiplexer::demultiplexer demux{};
  demux.read_queue.emplace();
  demux.write_queue.emplace();

  socket_handle socket{AF_INET, SOCK_STREAM, IPPROTO_TCP};
  demux.socket = &socket;

  short revents = (POLLERR | POLLOUT | POLLIN);
  auto queues = prepare_handles(revents, demux);

  EXPECT_EQ(queues.size(), 2);
  EXPECT_EQ(queues[0].size(), 1);
  EXPECT_EQ(queues[1].size(), 1);

  demux.read_queue.emplace();
  revents = (POLLIN | POLLOUT);
  queues = prepare_handles(revents, demux);

  EXPECT_EQ(queues.size(), 1);
  EXPECT_EQ(queues[0].size(), 1);

  demux.write_queue.emplace();
  queues = prepare_handles(revents, demux);

  EXPECT_EQ(queues.size(), 1);
  EXPECT_EQ(queues[0].size(), 1);
}

TEST_F(PollTriggersTest, MakeReadyQueuesTest)
{
  std::vector<pollfd> list{{.fd = 1, .events = POLLIN, .revents = 0}};
  std::map<int, poll_multiplexer::demultiplexer> demux{};

  auto ready = make_ready_queues(list, demux);
  EXPECT_TRUE(ready.empty());
}

TEST_F(PollTriggersTest, SubmitTest)
{
  using trigger = execution_trigger;
  using socket_handle = ::io::socket::socket_handle;

  std::array<int, 2> sockets{};
  int status = ::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data());
  ASSERT_EQ(status, 0);

  std::array<char, 2> buf1{};
  std::array<char, 2> buf2{};

  auto read_socket = std::make_shared<socket_handle>(sockets[0]);
  auto write_socket = std::make_shared<socket_handle>(sockets[1]);

  stdexec::sender auto read = triggers.set(read_socket, trigger::READ, [&] {
    return std::optional(::read(sockets[0], buf1.data(), 1));
  });
  write(sockets[1], "a", 1);
  triggers.wait_for(0);
  EXPECT_EQ(buf1[0], 'a');

  stdexec::sender auto write = triggers.set(write_socket, trigger::WRITE, [&] {
    return std::optional(::write(sockets[1], "b", 1));
  });
  triggers.wait_for(0);
  ::read(sockets[0], buf2.data(), 1);
  EXPECT_EQ(buf2[0], 'b');
}

TEST_F(PollTriggersTest, AsyncAcceptTest)
{
  using ::io::socket::make_address;

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

  stdexec::sender auto async_accept = ::io::accept(dialog, address);
  triggers1.wait_for(0);
  auto [result] = stdexec::sync_wait(std::move(async_accept)).value();
  auto [accept_handle, accept_address] = std::move(result);
  EXPECT_NE(accept_handle, -1);

  auto client_address = make_address<struct sockaddr_in>();
  auto client_addr = ::io::getsockname(client, client_address);
  ASSERT_EQ(client_addr, client_address);
  EXPECT_EQ(client_address, accept_address);
}
