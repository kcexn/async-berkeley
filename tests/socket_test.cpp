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

#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace io::socket;

class SocketTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketTest, BindTagInvoke) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  int result = ::io::bind(
      handle, reinterpret_cast<const sockaddr_type *>(&addr), sizeof(addr));

  EXPECT_EQ(result, 0);
}

TEST_F(SocketTest, BindTagInvokeWithSocketAddress) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  socket_address socket_addr(reinterpret_cast<const sockaddr_type *>(&addr),
                             sizeof(addr));

  int result = ::io::bind(handle, socket_addr);

  EXPECT_EQ(result, 0);
}

TEST_F(SocketTest, BindTagInvokeWithSocketAddressInvalidSocket) {
  socket_handle invalid_handle;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  socket_address socket_addr(reinterpret_cast<const sockaddr_type *>(&addr),
                             sizeof(addr));

  int result = ::io::bind(invalid_handle, socket_addr);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, ListenTagInvoke) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  int bind_result = ::io::bind(
      handle, reinterpret_cast<const sockaddr_type *>(&addr), sizeof(addr));
  ASSERT_EQ(bind_result, 0);

  int listen_result = ::io::listen(handle, 5);

  EXPECT_EQ(listen_result, 0);
}

TEST_F(SocketTest, ListenTagInvokeWithDifferentBacklogs) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr1{}, addr2{};
  addr1.sin_family = addr2.sin_family = AF_INET;
  addr1.sin_addr.s_addr = addr2.sin_addr.s_addr = INADDR_ANY;
  addr1.sin_port = addr2.sin_port = 0;

  ASSERT_EQ(::io::bind(handle1, reinterpret_cast<const sockaddr_type *>(&addr1),
                       sizeof(addr1)),
            0);
  ASSERT_EQ(::io::bind(handle2, reinterpret_cast<const sockaddr_type *>(&addr2),
                       sizeof(addr2)),
            0);

  EXPECT_EQ(::io::listen(handle1, 1), 0);
  EXPECT_EQ(::io::listen(handle2, SOMAXCONN), 0);
}

TEST_F(SocketTest, ListenTagInvokeOnInvalidSocket) {
  socket_handle invalid_handle;

  int listen_result = ::io::listen(invalid_handle, 5);

  EXPECT_EQ(listen_result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, ListenTagInvokeOnDatagramSocket) {
  socket_handle dgram_handle(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  ASSERT_EQ(::io::bind(dgram_handle,
                       reinterpret_cast<const sockaddr_type *>(&addr),
                       sizeof(addr)),
            0);

  int listen_result = ::io::listen(dgram_handle, 5);

  EXPECT_EQ(listen_result, -1);
  EXPECT_EQ(errno, EOPNOTSUPP);
}

TEST_F(SocketTest, ConnectTagInvokeSuccessfulConnection) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);

  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  std::thread server_thread([&server_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    native_socket_type accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);
    if (accepted_socket != -1) {
      ::close(accepted_socket);
    }
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  int result = ::io::connect(
      client_socket, reinterpret_cast<const sockaddr_type *>(&server_addr),
      sizeof(server_addr));

  server_thread.join();

  EXPECT_EQ(result, 0);
}

TEST_F(SocketTest, ConnectTagInvokeConnectionRefused) {
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(1);

  int result = ::io::connect(client_socket,
                             reinterpret_cast<const sockaddr_type *>(&addr),
                             sizeof(addr));

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, ECONNREFUSED);
}

TEST_F(SocketTest, ConnectTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(80);

  int result = ::io::connect(invalid_socket,
                             reinterpret_cast<const sockaddr_type *>(&addr),
                             sizeof(addr));

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, ConnectTagInvokeWithSocketAddressSuccessfulConnection) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);

  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  socket_address connect_addr(
      reinterpret_cast<const sockaddr_type *>(&server_addr),
      sizeof(server_addr));

  std::thread server_thread([&server_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    native_socket_type accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);
    if (accepted_socket != -1) {
      ::close(accepted_socket);
    }
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  int result = ::io::connect(client_socket, connect_addr);

  server_thread.join();

  EXPECT_EQ(result, 0);
}

TEST_F(SocketTest, ConnectTagInvokeWithSocketAddressConnectionRefused) {
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(1);

  socket_address connect_addr(reinterpret_cast<const sockaddr_type *>(&addr),
                              sizeof(addr));

  int result = ::io::connect(client_socket, connect_addr);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, ECONNREFUSED);
}

TEST_F(SocketTest, ConnectTagInvokeWithSocketAddressInvalidSocket) {
  socket_handle invalid_socket;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(80);

  socket_address connect_addr(reinterpret_cast<const sockaddr_type *>(&addr),
                              sizeof(addr));

  int result = ::io::connect(invalid_socket, connect_addr);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, ConnectTagInvokeWithSocketAddressInvalidAddress) {
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(0x00000000);
  addr.sin_port = htons(0);

  socket_address connect_addr(reinterpret_cast<const sockaddr_type *>(&addr),
                              sizeof(addr));

  int result = ::io::connect(client_socket, connect_addr);

  EXPECT_EQ(result, -1);
  EXPECT_TRUE(errno == EADDRNOTAVAIL || errno == ENETUNREACH ||
              errno == ECONNREFUSED);
}

TEST_F(SocketTest, AcceptTagInvokeSuccessfulAccept) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);

  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  std::thread client_thread([&server_addr] {
    socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ::io::connect(client_socket,
                  reinterpret_cast<const sockaddr_type *>(&server_addr),
                  sizeof(server_addr));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  native_socket_type accepted_socket = ::io::accept(
      server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
      &client_addr_len);

  client_thread.join();

  EXPECT_NE(accepted_socket, -1);
  EXPECT_EQ(client_addr.sin_family, AF_INET);
  EXPECT_GT(client_addr_len, 0);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, AcceptTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  native_socket_type result = ::io::accept(
      invalid_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
      &client_addr_len);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, AcceptTagInvokeNotListening) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  native_socket_type result =
      ::io::accept(socket, reinterpret_cast<sockaddr_type *>(&client_addr),
                   &client_addr_len);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EINVAL);
}

TEST_F(SocketTest, AcceptTagInvokeWithSocketAddressSuccessfulAccept) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);

  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  std::thread client_thread([&server_addr] {
    socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ::io::connect(client_socket,
                  reinterpret_cast<const sockaddr_type *>(&server_addr),
                  sizeof(server_addr));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  auto [accepted_handle, client_addr] = ::io::accept(server_socket);

  client_thread.join();

  EXPECT_TRUE(static_cast<bool>(accepted_handle));
  EXPECT_NE(static_cast<native_socket_type>(accepted_handle), -1);

  const auto *addr_ptr =
      reinterpret_cast<const sockaddr_in *>(client_addr.data());
  EXPECT_EQ(addr_ptr->sin_family, AF_INET);
  EXPECT_GT(*client_addr.size(), 0);
}

TEST_F(SocketTest, AcceptTagInvokeWithSocketAddressInvalidSocket) {
  socket_handle invalid_socket;

  EXPECT_THROW(::io::accept(invalid_socket), std::system_error);
}

TEST_F(SocketTest, AcceptTagInvokeWithSocketAddressNotListening) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  EXPECT_THROW(::io::accept(socket), std::system_error);
}

TEST_F(SocketTest, AcceptTagInvokeWithSocketAddressWithProvidedAddress) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);

  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  std::thread client_thread([&server_addr] {
    socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ::io::connect(client_socket,
                  reinterpret_cast<const sockaddr_type *>(&server_addr),
                  sizeof(server_addr));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  sockaddr_storage initial_storage{};
  socket_address provided_addr(
      reinterpret_cast<const sockaddr_type *>(&initial_storage),
      sizeof(initial_storage));

  auto [accepted_handle, client_addr] =
      ::io::accept(server_socket, provided_addr);

  client_thread.join();

  EXPECT_TRUE(static_cast<bool>(accepted_handle));
  EXPECT_NE(static_cast<native_socket_type>(accepted_handle), -1);

  const auto *addr_ptr =
      reinterpret_cast<const sockaddr_in *>(client_addr.data());
  EXPECT_EQ(addr_ptr->sin_family, AF_INET);
  EXPECT_GT(*client_addr.size(), 0);
}

TEST_F(SocketTest, SendmsgTagInvokeBasicSend) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  native_socket_type accepted_socket = -1;
  std::thread server_thread([&server_socket, &accepted_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);

    if (accepted_socket != -1) {
      std::array<char, 1024> buffer{};
      recv(accepted_socket, buffer.data(), buffer.size(), 0);
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  const char *message = "Hello, World!";
  struct iovec iov {};
  iov.iov_base = const_cast<char *>(message);
  iov.iov_len = strlen(message);

  struct msghdr msg {};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  ssize_t result = ::io::sendmsg(client_socket, &msg, 0);

  server_thread.join();

  EXPECT_EQ(result, static_cast<ssize_t>(strlen(message)));

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, SendmsgTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  const char *message = "Hello";
  struct iovec iov {};
  iov.iov_base = const_cast<char *>(message);
  iov.iov_len = strlen(message);

  struct msghdr msg {};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  ssize_t result = ::io::sendmsg(invalid_socket, &msg, 0);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, RecvmsgTagInvokeBasicReceive) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  const char *test_message = "Test Message";
  native_socket_type accepted_socket = -1;

  std::thread server_thread([&server_socket, &accepted_socket, test_message] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);

    if (accepted_socket != -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      send(accepted_socket, test_message, strlen(test_message), 0);
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  std::array<char, 1024> buffer{};
  struct iovec iov {};
  iov.iov_base = buffer.data();
  iov.iov_len = buffer.size();

  struct msghdr msg {};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  ssize_t result = ::io::recvmsg(client_socket, &msg, 0);

  server_thread.join();

  EXPECT_EQ(result, static_cast<ssize_t>(strlen(test_message)));
  EXPECT_EQ(strncmp(buffer.data(), test_message, strlen(test_message)), 0);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, RecvmsgTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  std::array<char, 1024> buffer{};
  struct iovec iov {};
  iov.iov_base = buffer.data();
  iov.iov_len = buffer.size();

  struct msghdr msg {};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  ssize_t result = ::io::recvmsg(invalid_socket, &msg, 0);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, GetsockoptTagInvokeBasicGet) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  int socket_type = 0;
  socklen_t len = sizeof(socket_type);
  int result =
      ::io::getsockopt(socket, SOL_SOCKET, SO_TYPE, &socket_type, &len);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(socket_type, SOCK_STREAM);
  EXPECT_EQ(len, sizeof(socket_type));
}

TEST_F(SocketTest, GetsockoptTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  int socket_type = 0;
  socklen_t len = sizeof(socket_type);
  int result =
      ::io::getsockopt(invalid_socket, SOL_SOCKET, SO_TYPE, &socket_type, &len);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, SetsockoptTagInvokeBasicSet) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  int reuse_addr = 1;
  int result = ::io::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
                                sizeof(reuse_addr));

  EXPECT_EQ(result, 0);

  int retrieved_value = 0;
  socklen_t len = sizeof(retrieved_value);
  ASSERT_EQ(::io::getsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &retrieved_value,
                             &len),
            0);
  EXPECT_EQ(retrieved_value, 1);
}

TEST_F(SocketTest, SetsockoptTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  int reuse_addr = 1;
  int result = ::io::setsockopt(invalid_socket, SOL_SOCKET, SO_REUSEADDR,
                                &reuse_addr, sizeof(reuse_addr));

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, GetsocknameTagInvokeBasicGet) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in bind_addr{};
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = INADDR_ANY;
  bind_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(socket,
                       reinterpret_cast<const sockaddr_type *>(&bind_addr),
                       sizeof(bind_addr)),
            0);

  sockaddr_in retrieved_addr{};
  socklen_t addr_len = sizeof(retrieved_addr);
  int result = ::io::getsockname(
      socket, reinterpret_cast<sockaddr_type *>(&retrieved_addr), &addr_len);

  EXPECT_EQ(result, 0);
  EXPECT_EQ(retrieved_addr.sin_family, AF_INET);
  EXPECT_EQ(addr_len, sizeof(retrieved_addr));
}

TEST_F(SocketTest, GetsocknameTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  sockaddr_in addr{};
  socklen_t addr_len = sizeof(addr);
  int result = ::io::getsockname(
      invalid_socket, reinterpret_cast<sockaddr_type *>(&addr), &addr_len);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, GetpeernameTagInvokeSuccessfulGet) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  native_socket_type accepted_socket = -1;
  std::thread server_thread([&server_socket, &accepted_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);
    if (accepted_socket != -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  sockaddr_in peer_addr{};
  socklen_t peer_addr_len = sizeof(peer_addr);
  int result = ::io::getpeername(client_socket,
                                 reinterpret_cast<sockaddr_type *>(&peer_addr),
                                 &peer_addr_len);

  server_thread.join();

  EXPECT_EQ(result, 0);
  EXPECT_EQ(peer_addr.sin_family, AF_INET);
  EXPECT_EQ(peer_addr.sin_addr.s_addr, htonl(INADDR_LOOPBACK));
  EXPECT_GT(peer_addr_len, 0);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, GetpeernameTagInvokeNotConnected) {
  socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in peer_addr{};
  socklen_t peer_addr_len = sizeof(peer_addr);
  int result = ::io::getpeername(
      socket, reinterpret_cast<sockaddr_type *>(&peer_addr), &peer_addr_len);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, ENOTCONN);
}

TEST_F(SocketTest, ShutdownTagInvokeSuccessfulShutdown) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  native_socket_type accepted_socket = -1;
  std::thread server_thread([&server_socket, &accepted_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);
    if (accepted_socket != -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  int result = ::io::shutdown(client_socket, SHUT_RDWR);

  server_thread.join();

  EXPECT_EQ(result, 0);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, ShutdownTagInvokeInvalidSocket) {
  socket_handle invalid_socket;

  int result = ::io::shutdown(invalid_socket, SHUT_RDWR);

  EXPECT_EQ(result, -1);
  EXPECT_EQ(errno, EBADF);
}

TEST_F(SocketTest, ShutdownTagInvokePartialShutdown) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  native_socket_type accepted_socket = -1;
  std::thread server_thread([&server_socket, &accepted_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);
    if (accepted_socket != -1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  int result_wr = ::io::shutdown(client_socket, SHUT_WR);
  int result_rd = ::io::shutdown(client_socket, SHUT_RD);

  server_thread.join();

  EXPECT_EQ(result_wr, 0);
  EXPECT_EQ(result_rd, 0);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}

TEST_F(SocketTest, SendmsgTagInvokeWithSocketMessage) {
  socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = 0;

  ASSERT_EQ(::io::bind(server_socket,
                       reinterpret_cast<const sockaddr_type *>(&server_addr),
                       sizeof(server_addr)),
            0);
  ASSERT_EQ(::io::listen(server_socket, 5), 0);

  socklen_t addr_len = sizeof(server_addr);
  ASSERT_EQ(::io::getsockname(server_socket,
                              reinterpret_cast<sockaddr_type *>(&server_addr),
                              &addr_len),
            0);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  native_socket_type accepted_socket = -1;
  std::thread server_thread([&server_socket, &accepted_socket] {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    accepted_socket = ::io::accept(
        server_socket, reinterpret_cast<sockaddr_type *>(&client_addr),
        &client_addr_len);

    if (accepted_socket != -1) {
      std::array<char, 1024> buffer{};
      recv(accepted_socket, buffer.data(), buffer.size(), 0);
    }
  });

  ASSERT_EQ(::io::connect(client_socket,
                          reinterpret_cast<const sockaddr_type *>(&server_addr),
                          sizeof(server_addr)),
            0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  socket_message msg;

  std::array<char, 14> message_data;
  std::strcpy(message_data.data(), "Hello, World!");

  msg.emplace_back(message_data);
  msg.set_flags(0);

  std::streamsize result = ::io::sendmsg(client_socket, msg);

  server_thread.join();

  EXPECT_EQ(result, 14);

  if (accepted_socket != -1) {
    ::close(accepted_socket);
  }
}
