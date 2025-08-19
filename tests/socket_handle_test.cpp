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
#include "../src/socket/socket_address.hpp"
#include "../src/socket/socket_handle.hpp"
#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace io::socket;

class SocketHandleTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketHandleTest, DefaultConstruction) {
  socket_handle handle;

  EXPECT_FALSE(static_cast<bool>(handle));
  EXPECT_TRUE(handle == INVALID_SOCKET);
}

TEST_F(SocketHandleTest, ValidSocketCreation) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_FALSE(handle == INVALID_SOCKET);
}

TEST_F(SocketHandleTest, InvalidSocketCreation) {
  EXPECT_THROW({ socket_handle handle(-1, -1, -1); }, std::system_error);
}

TEST_F(SocketHandleTest, NativeSocketConstruction) {
  int native_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(native_socket, -1);

  socket_handle handle(native_socket);

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_TRUE(handle == native_socket);
}

TEST_F(SocketHandleTest, MoveConstructor) {
  socket_handle original(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  socket_handle moved(std::move(original));

  EXPECT_TRUE(static_cast<bool>(moved));
  EXPECT_FALSE(moved == INVALID_SOCKET);

  EXPECT_FALSE(static_cast<bool>(original));
  EXPECT_TRUE(original == INVALID_SOCKET);
}

TEST_F(SocketHandleTest, MoveAssignment) {
  socket_handle original(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  socket_handle target;
  target = std::move(original);

  EXPECT_TRUE(static_cast<bool>(target));
  EXPECT_FALSE(target == INVALID_SOCKET);

  EXPECT_FALSE(static_cast<bool>(original));
  EXPECT_TRUE(original == INVALID_SOCKET);
}

TEST_F(SocketHandleTest, SelfMoveAssignment) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto original_socket = static_cast<native_socket_type>(handle);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
  handle = std::move(handle);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_FALSE(handle == INVALID_SOCKET);
  // Self-move should preserve the original socket
  EXPECT_EQ(static_cast<native_socket_type>(handle), original_socket);
}

TEST_F(SocketHandleTest, SwapFunction) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  socket_handle original1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle original2(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  swap(handle1, handle2);

  EXPECT_TRUE(static_cast<bool>(handle1));
  EXPECT_TRUE(static_cast<bool>(handle2));
  EXPECT_NE(handle1, handle2);
}

TEST_F(SocketHandleTest, SwapWithInvalidSocket) {
  socket_handle valid_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle invalid_handle;

  swap(valid_handle, invalid_handle);

  EXPECT_FALSE(static_cast<bool>(valid_handle));
  EXPECT_TRUE(static_cast<bool>(invalid_handle));
  EXPECT_TRUE(valid_handle == INVALID_SOCKET);
  EXPECT_FALSE(invalid_handle == INVALID_SOCKET);
}

TEST_F(SocketHandleTest, SwapWithSelf) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Store the original socket for comparison
  auto original_socket = static_cast<native_socket_type>(handle);

  // Self swap should not corrupt data and should not hang
  swap(handle, handle);

  // Handle should remain valid and unchanged
  EXPECT_TRUE(static_cast<bool>(handle));
  EXPECT_FALSE(handle == INVALID_SOCKET);
  EXPECT_EQ(static_cast<native_socket_type>(handle), original_socket);
}

TEST_F(SocketHandleTest, ComparisonOperators) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle invalid_handle;

  auto cmp = handle1 <=> handle2;
  if (cmp < 0) {
    EXPECT_TRUE(handle1 < handle2);
    EXPECT_FALSE(handle1 > handle2);
    EXPECT_TRUE(handle1 <= handle2);
    EXPECT_FALSE(handle1 >= handle2);
    EXPECT_TRUE(handle1 != handle2);
  } else if (cmp > 0) {
    EXPECT_FALSE(handle1 < handle2);
    EXPECT_TRUE(handle1 > handle2);
    EXPECT_FALSE(handle1 <= handle2);
    EXPECT_TRUE(handle1 >= handle2);
    EXPECT_TRUE(handle1 != handle2);
  } else {
    EXPECT_FALSE(handle1 < handle2);
    EXPECT_FALSE(handle1 > handle2);
    EXPECT_TRUE(handle1 <= handle2);
    EXPECT_TRUE(handle1 >= handle2);
    EXPECT_FALSE(handle1 != handle2);
  }

  EXPECT_TRUE(invalid_handle < handle1);
  EXPECT_TRUE(invalid_handle < handle2);
  EXPECT_TRUE(invalid_handle != handle1);
  EXPECT_TRUE(invalid_handle != handle2);
}

TEST_F(SocketHandleTest, CommutativeEquality) {
  int native_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(native_socket, -1);

  socket_handle handle(native_socket);

  EXPECT_TRUE(handle == native_socket);
  EXPECT_TRUE(native_socket == handle);

  // Test consistency with comparison
  EXPECT_TRUE(static_cast<native_socket_type>(handle) == native_socket);

  EXPECT_TRUE(handle == INVALID_SOCKET || native_socket == handle);
  EXPECT_TRUE(INVALID_SOCKET == handle || handle == native_socket);
}

TEST_F(SocketHandleTest, ThreadSafetyAccess) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  constexpr int num_threads = 10;
  constexpr int operations_per_thread = 100;
  std::vector<std::thread> threads;
  std::vector<bool> results(num_threads * operations_per_thread, false);

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&handle, &results, t, operations_per_thread] {
      for (int i = 0; i < operations_per_thread; ++i) {
        int index = t * operations_per_thread + i;

        bool is_valid = static_cast<bool>(handle);
        bool is_not_invalid = !(handle == INVALID_SOCKET);
        // Test access to socket member
        bool consistent = (static_cast<native_socket_type>(handle) !=
                           INVALID_SOCKET) == is_valid;

        results[index] = is_valid && is_not_invalid && consistent;

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

  // Store original socket values to verify they remain valid
  auto original1 = static_cast<native_socket_type>(handle1);
  auto original2 = static_cast<native_socket_type>(handle2);

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

  EXPECT_TRUE(static_cast<bool>(handle1));
  EXPECT_TRUE(static_cast<bool>(handle2));
  EXPECT_NE(handle1, handle2);

  // Verify one of the original sockets is in each handle
  auto current1 = static_cast<native_socket_type>(handle1);
  auto current2 = static_cast<native_socket_type>(handle2);
  EXPECT_TRUE((current1 == original1 && current2 == original2) ||
              (current1 == original2 && current2 == original1));
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
  int native_socket = create_and_get_socket();
  EXPECT_TRUE(is_socket_valid(native_socket));

  {
    socket_handle handle(native_socket);
    EXPECT_TRUE(static_cast<bool>(handle));
  }

  EXPECT_FALSE(is_socket_valid(native_socket));
}

TEST_F(SocketHandleTest, SocketAccess) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Test access to socket value through explicit conversion
  auto socket_value = static_cast<native_socket_type>(handle);
  EXPECT_NE(socket_value, INVALID_SOCKET);

  // Test operations are consistent with bool operator
  EXPECT_EQ(static_cast<bool>(handle), socket_value != INVALID_SOCKET);
}

TEST_F(SocketHandleTest, ExplicitNativeSocketTypeConversion) {
  // Test with valid socket
  socket_handle valid_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Explicit conversion should work
  native_socket_type native_socket =
      static_cast<native_socket_type>(valid_handle);
  EXPECT_NE(native_socket, INVALID_SOCKET);

  // Should be the same value when converted multiple times
  native_socket_type native_socket2 =
      static_cast<native_socket_type>(valid_handle);
  EXPECT_EQ(native_socket, native_socket2);

  // Test with invalid socket
  socket_handle invalid_handle;
  native_socket_type invalid_native =
      static_cast<native_socket_type>(invalid_handle);
  EXPECT_EQ(invalid_native, INVALID_SOCKET);

  // Test that explicit conversion is thread-safe
  std::atomic<bool> conversion_succeeded{true};
  std::thread t([&valid_handle, &conversion_succeeded] {
    try {
      for (int i = 0; i < 100; ++i) {
        native_socket_type socket_val =
            static_cast<native_socket_type>(valid_handle);
        if (socket_val == INVALID_SOCKET) {
          conversion_succeeded = false;
          break;
        }
      }
    } catch (...) {
      conversion_succeeded = false;
    }
  });

  t.join();
  EXPECT_TRUE(conversion_succeeded.load());
}

TEST_F(SocketHandleTest, ComparisonOperationsWork) {
  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Test that comparison operators work
  auto socket1 = static_cast<native_socket_type>(handle1);
  auto socket2 = static_cast<native_socket_type>(handle2);

  EXPECT_TRUE(handle1 == socket1);
  EXPECT_TRUE(handle2 == socket2);
  EXPECT_TRUE(socket1 == handle1);
  EXPECT_TRUE(socket2 == handle2);
}

TEST_F(SocketHandleTest, InvalidSocketValidationInConstructor) {
  // Test various invalid socket values
  EXPECT_THROW(socket_handle handle(INVALID_SOCKET), std::system_error);
  EXPECT_THROW(socket_handle handle(-2), std::system_error);
  EXPECT_THROW(socket_handle handle(-999), std::system_error);

  // Test with closed socket
  int closed_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(closed_socket, -1);
  ::close(closed_socket);
  EXPECT_THROW(socket_handle handle(closed_socket), std::system_error);
}

TEST_F(SocketHandleTest, SocketClosureBehaviorInMoveOperations) {
  // Test move constructor doesn't close original socket
  int native_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(native_socket, -1);

  {
    socket_handle original(native_socket);
    socket_handle moved(std::move(original));

    // Original should now be invalid, but socket should still be open
    EXPECT_FALSE(static_cast<bool>(original));
    EXPECT_TRUE(static_cast<bool>(moved));

    // Verify socket is still valid by checking with getsockopt
    int type = 0;
    socklen_t len = sizeof(type);
    EXPECT_EQ(::getsockopt(native_socket, SOL_SOCKET, SO_TYPE, &type, &len), 0);
  }

  // Socket should be closed when moved handle is destroyed
  int type = 0;
  socklen_t len = sizeof(type);
  EXPECT_EQ(::getsockopt(native_socket, SOL_SOCKET, SO_TYPE, &type, &len), -1);
}

TEST_F(SocketHandleTest, ExceptionSafetyGuarantees) {
  // Test that failed socket creation doesn't leak resources
  try {
    socket_handle handle(-1, -1, -1);
    FAIL() << "Expected exception not thrown";
  } catch (const std::system_error &) {
    // Expected exception
  }

  // Test that move assignment swaps sockets (doesn't close, but transfers
  // ownership)
  socket_handle valid_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto original_valid = static_cast<native_socket_type>(valid_handle);

  socket_handle target(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  auto original_target = static_cast<native_socket_type>(target);

  target = std::move(valid_handle);

  // After move assignment, target should have original_valid socket
  EXPECT_EQ(static_cast<native_socket_type>(target), original_valid);

  // valid_handle should now have original_target socket (via swap)
  EXPECT_EQ(static_cast<native_socket_type>(valid_handle), original_target);

  // Both sockets should still be valid until destructors are called
  int type = 0;
  socklen_t len = sizeof(type);
  EXPECT_EQ(::getsockopt(original_valid, SOL_SOCKET, SO_TYPE, &type, &len), 0);
  EXPECT_EQ(::getsockopt(original_target, SOL_SOCKET, SO_TYPE, &type, &len), 0);
}

TEST_F(SocketHandleTest, EdgeCasesInComparisonOperations) {
  socket_handle valid_handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle invalid_handle;
  socket_handle another_invalid;

  // Test invalid handles compare equal
  EXPECT_TRUE(invalid_handle == another_invalid);
  EXPECT_TRUE(another_invalid == invalid_handle);
  EXPECT_FALSE(invalid_handle != another_invalid);

  // Test three-way comparison consistency
  auto cmp1 = invalid_handle <=> another_invalid;
  auto cmp2 = another_invalid <=> invalid_handle;
  EXPECT_EQ(cmp1, std::strong_ordering::equal);
  EXPECT_EQ(cmp2, std::strong_ordering::equal);

  // Test comparison with native socket values
  EXPECT_TRUE(invalid_handle == INVALID_SOCKET);
  EXPECT_TRUE(INVALID_SOCKET == invalid_handle);
  EXPECT_FALSE(valid_handle == INVALID_SOCKET);
  EXPECT_FALSE(INVALID_SOCKET == valid_handle);

  // Test ordering with invalid sockets
  EXPECT_TRUE(invalid_handle <= valid_handle);
  EXPECT_TRUE(invalid_handle < valid_handle);
  EXPECT_FALSE(invalid_handle > valid_handle);
  EXPECT_FALSE(invalid_handle >= valid_handle);
}

TEST_F(SocketHandleTest, SocketValidationUsingGetsockopt) {
  // Test that constructor validates socket using getsockopt
  int valid_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ASSERT_NE(valid_socket, -1);

  // Should succeed with valid socket
  EXPECT_NO_THROW(socket_handle handle(valid_socket));

  // Test with invalid file descriptor (not a socket)
  int pipe_fds[2];
  ASSERT_EQ(::pipe(pipe_fds), 0);

  // pipe file descriptor should fail socket validation
  EXPECT_THROW(socket_handle handle(pipe_fds[0]), std::system_error);

  ::close(pipe_fds[0]);
  ::close(pipe_fds[1]);
}

TEST_F(SocketHandleTest, AtomicOperationsAndMemoryOrdering) {
  socket_handle handle(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  auto original_socket = static_cast<native_socket_type>(handle);

  // Test that multiple concurrent reads return consistent values
  std::atomic<bool> all_consistent{true};
  std::vector<std::thread> readers;

  for (int i = 0; i < 10; ++i) {
    readers.emplace_back([&handle, &all_consistent, original_socket] {
      for (int j = 0; j < 1000; ++j) {
        auto current = static_cast<native_socket_type>(handle);
        if (current != original_socket) {
          all_consistent = false;
          break;
        }
      }
    });
  }

  for (auto &reader : readers) {
    reader.join();
  }

  EXPECT_TRUE(all_consistent.load());

  // Test that boolean conversion is consistent with native socket value
  std::atomic<bool> bool_consistent{true};
  std::thread bool_checker([&handle, &bool_consistent] {
    for (int i = 0; i < 1000; ++i) {
      bool is_valid = static_cast<bool>(handle);
      auto native = static_cast<native_socket_type>(handle);
      if (is_valid != (native != INVALID_SOCKET)) {
        bool_consistent = false;
        break;
      }
    }
  });

  bool_checker.join();
  EXPECT_TRUE(bool_consistent.load());
}

TEST_F(SocketHandleTest, StressTestConcurrentOperations) {
  constexpr int num_operations = 1000;
  constexpr int num_threads = 4;

  socket_handle handle1(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_handle handle2(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  std::atomic<int> successful_operations{0};
  std::vector<std::thread> threads;

  // Start concurrent operations - each thread does all types of operations
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&] {
      for (int i = 0; i < num_operations / num_threads; ++i) {
        try {
          // Mix different operations
          [[maybe_unused]] auto cmp = handle1 <=> handle2;
          [[maybe_unused]] auto native =
              static_cast<native_socket_type>(handle1);
          [[maybe_unused]] bool valid = static_cast<bool>(handle2);

          successful_operations.fetch_add(1, std::memory_order_relaxed);
        } catch (...) {
          // Unexpected exception
        }

        // Small delay to increase contention
        if (i % 50 == 0) {
          std::this_thread::yield();
        }
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  // Verify most operations completed successfully
  EXPECT_GE(successful_operations.load(), num_operations - 10);

  // Verify handles are still valid
  EXPECT_TRUE(static_cast<bool>(handle1));
  EXPECT_TRUE(static_cast<bool>(handle2));
}
