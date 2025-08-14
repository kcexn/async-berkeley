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

#include "../src/socket/locked_socket_ptr.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

using namespace iosched::socket;

class LockedSocketPtrTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT_NE(test_socket_, INVALID_SOCKET);
  }

  void TearDown() override {
    if (test_socket_ != INVALID_SOCKET) {
      ::close(test_socket_);
    }
  }

  native_socket_type test_socket_ = INVALID_SOCKET;
  std::mutex test_mutex_;
};

TEST_F(LockedSocketPtrTest, BasicConstruction) {
  std::unique_lock<std::mutex> lock(test_mutex_);
  locked_socket_ptr ptr(std::move(lock), test_socket_);

  EXPECT_EQ(*ptr, test_socket_);
  EXPECT_EQ(ptr.get(), &test_socket_);
}

TEST_F(LockedSocketPtrTest, DereferenceOperator) {
  std::unique_lock<std::mutex> lock(test_mutex_);
  locked_socket_ptr ptr(std::move(lock), test_socket_);

  native_socket_type &socket_ref = *ptr;
  EXPECT_EQ(socket_ref, test_socket_);
  EXPECT_EQ(&socket_ref, &test_socket_);
}

TEST_F(LockedSocketPtrTest, GetMethod) {
  std::unique_lock<std::mutex> lock(test_mutex_);
  locked_socket_ptr ptr(std::move(lock), test_socket_);

  native_socket_type *socket_ptr = ptr.get();
  EXPECT_EQ(socket_ptr, &test_socket_);
  EXPECT_EQ(*socket_ptr, test_socket_);
}

TEST_F(LockedSocketPtrTest, ModificationThroughPointer) {
  native_socket_type original_socket = test_socket_;
  native_socket_type new_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ASSERT_NE(new_socket, INVALID_SOCKET);

  {
    std::unique_lock<std::mutex> lock(test_mutex_);
    locked_socket_ptr ptr(std::move(lock), test_socket_);

    *ptr = new_socket;
    EXPECT_EQ(test_socket_, new_socket);
    EXPECT_NE(test_socket_, original_socket);
  }

  ::close(original_socket);
  test_socket_ = new_socket; // Update for proper cleanup in TearDown
}

TEST_F(LockedSocketPtrTest, LockIsHeldDuringLifetime) {
  std::atomic<bool> lock_acquired{false};
  std::atomic<bool> start_test{false};
  std::atomic<bool> ptr_destroyed{false};

  std::thread background_thread([&]() {
    while (!start_test) {
      std::this_thread::yield();
    }

    // Try to acquire the lock - should block while ptr exists
    std::unique_lock<std::mutex> lock(test_mutex_);
    lock_acquired = true;
  });

  {
    std::unique_lock<std::mutex> lock(test_mutex_);
    locked_socket_ptr ptr(std::move(lock), test_socket_);

    start_test = true;

    // Give background thread time to try acquiring lock
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Lock should not be acquired yet
    EXPECT_FALSE(lock_acquired.load());
  }

  ptr_destroyed = true;
  background_thread.join();

  // After ptr is destroyed, background thread should have acquired lock
  EXPECT_TRUE(lock_acquired.load());
}

TEST_F(LockedSocketPtrTest, InvalidSocketHandling) {
  native_socket_type invalid_socket = INVALID_SOCKET;

  std::unique_lock<std::mutex> lock(test_mutex_);
  locked_socket_ptr ptr(std::move(lock), invalid_socket);

  EXPECT_EQ(*ptr, INVALID_SOCKET);
  EXPECT_EQ(ptr.get(), &invalid_socket);
}

TEST_F(LockedSocketPtrTest, ScopeBasedLocking) {
  std::atomic<int> critical_section_count{0};
  std::atomic<bool> start_threads{false};

  auto worker = [&](int worker_id) {
    while (!start_threads) {
      std::this_thread::yield();
    }

    for (int i = 0; i < 5; ++i) {
      std::unique_lock<std::mutex> lock(test_mutex_);
      locked_socket_ptr ptr(std::move(lock), test_socket_);

      // Critical section - only one thread should access at a time
      int current_count = critical_section_count.load();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      critical_section_count = current_count + 1;

      // Verify socket access is still valid
      EXPECT_EQ(*ptr, test_socket_);
    }
  };

  std::thread thread1(worker, 1);
  std::thread thread2(worker, 2);

  start_threads = true;
  thread1.join();
  thread2.join();

  // Should have exactly 10 increments (5 per thread)
  EXPECT_EQ(critical_section_count.load(), 10);
}

class LockedSocketPtrCopyMoveTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT_NE(test_socket_, INVALID_SOCKET);
  }

  void TearDown() override {
    if (test_socket_ != INVALID_SOCKET) {
      ::close(test_socket_);
    }
  }

  native_socket_type test_socket_ = INVALID_SOCKET;
  std::mutex test_mutex_;
};

// Compile-time tests for deleted operations
TEST_F(LockedSocketPtrCopyMoveTest, CopyConstructorDeleted) {
  // This test ensures the copy constructor is deleted
  static_assert(!std::is_copy_constructible_v<locked_socket_ptr>);
}

TEST_F(LockedSocketPtrCopyMoveTest, CopyAssignmentDeleted) {
  // This test ensures the copy assignment operator is deleted
  static_assert(!std::is_copy_assignable_v<locked_socket_ptr>);
}

TEST_F(LockedSocketPtrCopyMoveTest, MoveConstructorDeleted) {
  // This test ensures the move constructor is deleted
  static_assert(!std::is_move_constructible_v<locked_socket_ptr>);
}

TEST_F(LockedSocketPtrCopyMoveTest, MoveAssignmentDeleted) {
  // This test ensures the move assignment operator is deleted
  static_assert(!std::is_move_assignable_v<locked_socket_ptr>);
}

class LockedSocketPtrPerformanceTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT_NE(test_socket_, INVALID_SOCKET);
  }

  void TearDown() override {
    if (test_socket_ != INVALID_SOCKET) {
      ::close(test_socket_);
    }
  }

  native_socket_type test_socket_ = INVALID_SOCKET;
  std::mutex test_mutex_;
};

TEST_F(LockedSocketPtrPerformanceTest, RapidCreationDestruction) {
  const int iterations = 1000;

  for (int i = 0; i < iterations; ++i) {
    std::unique_lock<std::mutex> lock(test_mutex_);
    locked_socket_ptr ptr(std::move(lock), test_socket_);

    // Basic operations
    EXPECT_EQ(*ptr, test_socket_);
    EXPECT_NE(ptr.get(), nullptr);
  }
}

TEST_F(LockedSocketPtrPerformanceTest, ConcurrentAccess) {
  const int num_threads = 4;
  const int iterations_per_thread = 100;
  std::atomic<int> total_accesses{0};

  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&]() {
      for (int i = 0; i < iterations_per_thread; ++i) {
        std::unique_lock<std::mutex> lock(test_mutex_);
        locked_socket_ptr ptr(std::move(lock), test_socket_);

        // Simulate some work with the socket
        volatile native_socket_type socket_copy = *ptr;
        (void)socket_copy; // Suppress unused variable warning

        total_accesses.fetch_add(1);
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(total_accesses.load(), num_threads * iterations_per_thread);
}