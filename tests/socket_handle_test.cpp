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

#include "../src/socket/socket_handle.hpp"
#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

using namespace iosched::socket;

class SocketHandleTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketHandleTest, DefaultConstruction) {
  socket_handle handle;

  EXPECT_FALSE(static_cast<bool>(handle));
  EXPECT_EQ(static_cast<native_socket_type>(handle), INVALID_SOCKET);
}

TEST_F(SocketHandleTest, ValidSocketCreation) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_NE(static_cast<native_socket_type>(handle), INVALID_SOCKET);
}

TEST_F(SocketHandleTest, InvalidSocketCreation) {
  EXPECT_THROW({ socket_handle handle(-1, -1, -1); }, std::system_error);
}

TEST_F(SocketHandleTest, NativeSocketConstruction) {
  int native_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(native_socket, -1);

  socket_handle handle(native_socket);

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_EQ(static_cast<native_socket_type>(handle), native_socket);
}

TEST_F(SocketHandleTest, MoveConstructor) {
  socket_handle original(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  native_socket_type original_socket =
      static_cast<native_socket_type>(original);

  socket_handle moved(std::move(original));

  EXPECT_TRUE(static_cast<bool>(moved));
  EXPECT_EQ(static_cast<native_socket_type>(moved), original_socket);

  EXPECT_FALSE(static_cast<bool>(original));
  EXPECT_EQ(static_cast<native_socket_type>(original), INVALID_SOCKET);
}

TEST_F(SocketHandleTest, MoveAssignment) {
  socket_handle original(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  native_socket_type original_socket =
      static_cast<native_socket_type>(original);

  socket_handle target;
  target = std::move(original);

  EXPECT_TRUE(static_cast<bool>(target));
  EXPECT_EQ(static_cast<native_socket_type>(target), original_socket);

  EXPECT_FALSE(static_cast<bool>(original));
  EXPECT_EQ(static_cast<native_socket_type>(original), INVALID_SOCKET);
}

TEST_F(SocketHandleTest, SelfMoveAssignment) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  native_socket_type original_socket = static_cast<native_socket_type>(handle);

  handle = std::move(handle);

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_EQ(static_cast<native_socket_type>(handle), original_socket);
}

TEST_F(SocketHandleTest, SwapFunction) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  native_socket_type socket1 = static_cast<native_socket_type>(handle1);
  native_socket_type socket2 = static_cast<native_socket_type>(handle2);

  swap(handle1, handle2);

  EXPECT_EQ(static_cast<native_socket_type>(handle1), socket2);
  EXPECT_EQ(static_cast<native_socket_type>(handle2), socket1);
}

TEST_F(SocketHandleTest, SwapWithInvalidSocket) {
  socket_handle valid_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle invalid_handle;

  native_socket_type valid_socket =
      static_cast<native_socket_type>(valid_handle);

  swap(valid_handle, invalid_handle);

  EXPECT_FALSE(static_cast<bool>(valid_handle));
  EXPECT_TRUE(static_cast<bool>(invalid_handle));
  EXPECT_EQ(static_cast<native_socket_type>(invalid_handle), valid_socket);
}

TEST_F(SocketHandleTest, ComparisonOperators) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle invalid_handle;

  native_socket_type socket1 = static_cast<native_socket_type>(handle1);
  native_socket_type socket2 = static_cast<native_socket_type>(handle2);

  auto cmp = handle1 <=> handle2;
  if (socket1 < socket2) {
    EXPECT_TRUE(handle1 < handle2);
    EXPECT_FALSE(handle1 > handle2);
    EXPECT_TRUE(handle1 <= handle2);
    EXPECT_FALSE(handle1 >= handle2);
    EXPECT_TRUE(handle1 != handle2);
    EXPECT_TRUE(cmp < 0);
  } else if (socket1 > socket2) {
    EXPECT_FALSE(handle1 < handle2);
    EXPECT_TRUE(handle1 > handle2);
    EXPECT_FALSE(handle1 <= handle2);
    EXPECT_TRUE(handle1 >= handle2);
    EXPECT_TRUE(handle1 != handle2);
    EXPECT_TRUE(cmp > 0);
  } else {
    EXPECT_FALSE(handle1 < handle2);
    EXPECT_FALSE(handle1 > handle2);
    EXPECT_TRUE(handle1 <= handle2);
    EXPECT_TRUE(handle1 >= handle2);
    EXPECT_FALSE(handle1 != handle2);
    EXPECT_TRUE(cmp == 0);
  }

  EXPECT_TRUE(invalid_handle < handle1);
  EXPECT_TRUE(invalid_handle < handle2);
  EXPECT_TRUE(invalid_handle != handle1);
  EXPECT_TRUE(invalid_handle != handle2);
}

TEST_F(SocketHandleTest, ThreadSafetyAccess) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  native_socket_type expected_socket = static_cast<native_socket_type>(handle);

  constexpr int num_threads = 10;
  constexpr int operations_per_thread = 100;
  std::vector<std::thread> threads;
  std::vector<bool> results(num_threads * operations_per_thread, false);

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back(
        [&handle, expected_socket, &results, t, operations_per_thread] {
          for (int i = 0; i < operations_per_thread; ++i) {
            int index = t * operations_per_thread + i;

            native_socket_type socket_value =
                static_cast<native_socket_type>(handle);
            bool is_valid = static_cast<bool>(handle);

            results[index] = (socket_value == expected_socket) && is_valid;

            std::this_thread::sleep_for(std::chrono::microseconds(1));
          }
        });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_TRUE(std::all_of(results.begin(), results.end(),
                          [](bool result) { return result; }));
}

TEST_F(SocketHandleTest, ThreadSafetyComparison) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  constexpr int num_threads = 10;
  constexpr int operations_per_thread = 100;
  std::vector<std::thread> threads;
  std::atomic<int> successful_operations{0};

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back(
        [&handle1, &handle2, &successful_operations, operations_per_thread] {
          for (int i = 0; i < operations_per_thread; ++i) {
            try {
              [[maybe_unused]] auto result = handle1 <=> handle2;
              successful_operations.fetch_add(1, std::memory_order_relaxed);
            } catch (...) {
              // Unexpected exception
            }

            std::this_thread::sleep_for(std::chrono::microseconds(1));
          }
        });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(successful_operations.load(), num_threads * operations_per_thread);
}

TEST_F(SocketHandleTest, ThreadSafetySwap) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  native_socket_type original_socket1 =
      static_cast<native_socket_type>(handle1);
  native_socket_type original_socket2 =
      static_cast<native_socket_type>(handle2);

  constexpr int num_swaps = 1000;
  std::vector<std::thread> threads;

  for (int t = 0; t < 4; ++t) {
    threads.emplace_back([&handle1, &handle2] {
      for (int i = 0; i < num_swaps; ++i) {
        swap(handle1, handle2);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  native_socket_type final_socket1 = static_cast<native_socket_type>(handle1);
  native_socket_type final_socket2 = static_cast<native_socket_type>(handle2);

  EXPECT_TRUE(
      (final_socket1 == original_socket1 &&
       final_socket2 == original_socket2) ||
      (final_socket1 == original_socket2 && final_socket2 == original_socket1));
}

class SocketHandleRAIITest : public ::testing::Test {
protected:
  int create_and_get_socket() {
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  }

  bool is_socket_valid(int sock) {
    if (sock == -1)
      return false;

    int error = 0;
    socklen_t len = sizeof(error);
    int retval = ::getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
    return retval == 0;
  }
};

TEST_F(SocketHandleRAIITest, DestructorClosesSocket) {
  int native_socket;
  {
    socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    native_socket = static_cast<native_socket_type>(handle);
    EXPECT_TRUE(is_socket_valid(native_socket));
  }

  EXPECT_FALSE(is_socket_valid(native_socket));
}

TEST_F(SocketHandleRAIITest, MovePreservesSocket) {
  int native_socket;
  socket_handle moved_handle;

  {
    socket_handle original_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    native_socket = static_cast<native_socket_type>(original_handle);
    EXPECT_TRUE(is_socket_valid(native_socket));

    moved_handle = std::move(original_handle);
  }

  EXPECT_TRUE(is_socket_valid(native_socket));
  EXPECT_EQ(static_cast<native_socket_type>(moved_handle), native_socket);
}