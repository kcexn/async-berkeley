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

#include "../src/socket/socket_address.hpp"
#include "../src/socket/socket_message.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <numeric>
#include <sys/socket.h>
#include <thread>
#include <vector>

using namespace io::socket;

class SocketMessageTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

class SocketMessageRAIITest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test construction and basic properties
TEST_F(SocketMessageTest, DefaultConstruction) {
  socket_message msg;

  // Default constructed message should have initialized data
  auto addr = msg.address();
  auto buffers = msg.buffers();
  auto control = msg.control();
  int flags = msg.flags();

  EXPECT_TRUE(buffers.empty());
  EXPECT_TRUE(control.empty());
  EXPECT_EQ(flags, 0);
}

TEST_F(SocketMessageTest, CopyConstructionDeleted) {
  static_assert(!std::is_copy_constructible_v<socket_message>);
  static_assert(!std::is_copy_assignable_v<socket_message>);
}

TEST_F(SocketMessageTest, MoveConstruction) {
  socket_message msg1;

  // Set up some test data
  msg1.set_flags(42);

  std::vector<socket_buffer_type> buffers;
  std::array<char, 100> test_data{};
  test_data.fill('A');
  socket_buffer_type buf{test_data};
  buffers.push_back(buf);
  msg1.set_buffers(std::move(buffers));

  std::vector<char> control_data{'C', 'T', 'R', 'L'};
  msg1.set_control(std::move(control_data));

  // Move construct
  socket_message msg2 = std::move(msg1);

  // Verify data transferred
  EXPECT_EQ(msg2.flags(), 42);
  EXPECT_EQ(msg2.buffers().size(), 1);
  EXPECT_EQ(msg2.control().size(), 4);
  EXPECT_EQ(msg2.control()[0], 'C');
}

TEST_F(SocketMessageTest, MoveAssignment) {
  socket_message msg1;
  socket_message msg2;

  // Set up test data in msg1
  msg1.set_flags(123);

  std::vector<char> control_data{'T', 'E', 'S', 'T'};
  msg1.set_control(std::move(control_data));

  // Move assign
  msg2 = std::move(msg1);

  // Verify data transferred
  EXPECT_EQ(msg2.flags(), 123);
  EXPECT_EQ(msg2.control().size(), 4);
  EXPECT_EQ(msg2.control()[0], 'T');
}

TEST_F(SocketMessageTest, SelfMoveAssignment) {
  socket_message msg;
  msg.set_flags(456);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
  // Self assignment should not crash or corrupt data
  msg = std::move(msg);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
  EXPECT_EQ(msg.flags(), 456);
}

// Test socket address operations
TEST_F(SocketMessageTest, AddressOperations) {
  socket_message msg;

  // Create a test address
  socket_address test_addr;

  // Set address
  msg = test_addr;

  // Get address
  auto retrieved_addr = msg.address();

  // Both addresses should be default constructed with same size
  // Note: size() returns a pointer to socklen_t, so we compare the values
  EXPECT_EQ(*test_addr.size(), *retrieved_addr.size());
}

TEST_F(SocketMessageTest, AddressChaining) {
  socket_message msg;
  socket_address addr;

  // Test method chaining
  auto &result = (msg = addr);
  EXPECT_EQ(&result, &msg);
}

// Test buffer operations
TEST_F(SocketMessageTest, BufferOperations) {
  socket_message msg;

  std::vector<socket_buffer_type> buffers;

  // Create test buffers
  std::array<char, 50> data1{};
  std::array<char, 75> data2{};
  data1.fill('X');
  data2.fill('Y');

  socket_buffer_type buf1{data1};
  socket_buffer_type buf2{data2};

  buffers.push_back(buf1);
  buffers.push_back(buf2);

  // Set buffers
  msg.set_buffers(std::move(buffers));

  // Get buffers
  auto retrieved_buffers = msg.buffers();

  EXPECT_EQ(retrieved_buffers.size(), 2);
  EXPECT_EQ(retrieved_buffers[0].size(), 50);
  EXPECT_EQ(retrieved_buffers[1].size(), 75);
  EXPECT_EQ(retrieved_buffers[0].data(), data1.data());
  EXPECT_EQ(retrieved_buffers[1].data(), data2.data());
}

TEST_F(SocketMessageTest, BufferChaining) {
  socket_message msg;
  std::vector<socket_buffer_type> buffers;

  // Test method chaining
  auto &result = msg.set_buffers(std::move(buffers));
  EXPECT_EQ(&result, &msg);
}

TEST_F(SocketMessageTest, EmptyBuffers) {
  socket_message msg;

  std::vector<socket_buffer_type> empty_buffers;
  msg.set_buffers(std::move(empty_buffers));

  auto retrieved = msg.buffers();
  EXPECT_TRUE(retrieved.empty());
}

// Test control data operations
TEST_F(SocketMessageTest, ControlDataOperations) {
  socket_message msg;

  std::vector<char> control_data;
  control_data.assign({'H', 'E', 'L', 'L', 'O', '\0'});

  // Set control data
  msg.set_control(std::move(control_data));

  // Get control data
  auto retrieved = msg.control();

  EXPECT_EQ(retrieved.size(), 6);
  EXPECT_EQ(retrieved[0], 'H');
  EXPECT_EQ(retrieved[4], 'O');
  EXPECT_EQ(retrieved[5], '\0');
}

TEST_F(SocketMessageTest, ControlDataChaining) {
  socket_message msg;
  std::vector<char> control;

  // Test method chaining
  auto &result = msg.set_control(std::move(control));
  EXPECT_EQ(&result, &msg);
}

TEST_F(SocketMessageTest, EmptyControlData) {
  socket_message msg;

  std::vector<char> empty_control;
  msg.set_control(std::move(empty_control));

  auto retrieved = msg.control();
  EXPECT_TRUE(retrieved.empty());
}

TEST_F(SocketMessageTest, LargeControlData) {
  socket_message msg;

  std::vector<char> large_control(1024, 'Z');
  msg.set_control(std::move(large_control));

  auto retrieved = msg.control();
  EXPECT_EQ(retrieved.size(), 1024);
  EXPECT_TRUE(
      std::all_of(retrieved.begin(), retrieved.end(),
                  [](char control_char) { return control_char == 'Z'; }));
}

// Test flags operations
TEST_F(SocketMessageTest, FlagsOperations) {
  socket_message msg;

  // Test various flag values
  msg.set_flags(MSG_DONTWAIT);
  EXPECT_EQ(msg.flags(), MSG_DONTWAIT);

  msg.set_flags(MSG_PEEK);
  EXPECT_EQ(msg.flags(), MSG_PEEK);

  msg.set_flags(MSG_DONTWAIT | MSG_PEEK);
  EXPECT_EQ(msg.flags(), MSG_DONTWAIT | MSG_PEEK);

  msg.set_flags(0);
  EXPECT_EQ(msg.flags(), 0);

  msg.set_flags(-1);
  EXPECT_EQ(msg.flags(), -1);
}

TEST_F(SocketMessageTest, FlagsChaining) {
  socket_message msg;

  // Test method chaining
  auto &result = msg.set_flags(MSG_TRUNC);
  EXPECT_EQ(&result, &msg);
  EXPECT_EQ(msg.flags(), MSG_TRUNC);
}

// Test swap functionality
TEST_F(SocketMessageTest, SwapOperation) {
  socket_message msg1;
  socket_message msg2;

  // Set up different data in each message
  msg1.set_flags(100);
  std::vector<char> control1{'A', 'B', 'C'};
  msg1.set_control(std::move(control1));

  msg2.set_flags(200);
  std::vector<char> control2{'X', 'Y', 'Z', 'W'};
  msg2.set_control(std::move(control2));

  // Swap
  swap(msg1, msg2);

  // Verify swap occurred
  EXPECT_EQ(msg1.flags(), 200);
  EXPECT_EQ(msg2.flags(), 100);

  auto ctrl1 = msg1.control();
  auto ctrl2 = msg2.control();

  EXPECT_EQ(ctrl1.size(), 4);
  EXPECT_EQ(ctrl2.size(), 3);
  EXPECT_EQ(ctrl1[0], 'X');
  EXPECT_EQ(ctrl2[0], 'A');
}

TEST_F(SocketMessageTest, SwapWithSelf) {
  socket_message msg;
  msg.set_flags(789);

  std::vector<char> control{'S', 'E', 'L', 'F'};
  msg.set_control(std::move(control));

  // Self swap should not corrupt data
  swap(msg, msg);

  EXPECT_EQ(msg.flags(), 789);
  auto ctrl = msg.control();
  EXPECT_EQ(ctrl.size(), 4);
  EXPECT_EQ(ctrl[0], 'S');
}

// Test exchange methods
TEST_F(SocketMessageTest, ExchangeBuffers) {
  socket_message msg;

  std::vector<socket_buffer_type> initial_buffers;
  std::array<char, 50> initial_data{};
  initial_data.fill('I');
  socket_buffer_type initial_buf{initial_data};
  initial_buffers.push_back(initial_buf);
  msg.set_buffers(std::move(initial_buffers));

  // Exchange with new buffers
  std::vector<socket_buffer_type> new_buffers;
  std::array<char, 100> new_data{};
  new_data.fill('N');
  socket_buffer_type new_buf{new_data};
  new_buffers.push_back(new_buf);

  auto old_buffers = msg.exchange_buffers(std::move(new_buffers));

  // Verify exchange occurred
  auto current_buffers = msg.buffers();
  EXPECT_EQ(current_buffers.size(), 1);
  EXPECT_EQ(current_buffers[0].size(), 100);
  EXPECT_EQ(current_buffers[0].data(), new_data.data());

  // Verify old buffers returned
  EXPECT_EQ(old_buffers.size(), 1);
  EXPECT_EQ(old_buffers[0].size(), 50);
  EXPECT_EQ(old_buffers[0].data(), initial_data.data());
}

TEST_F(SocketMessageTest, ExchangeControl) {
  socket_message msg;

  std::vector<char> initial_control{'I', 'N', 'I', 'T'};
  msg.set_control(std::move(initial_control));

  // Exchange with new control data
  std::vector<char> new_control{'N', 'E', 'W', 'C', 'T', 'R', 'L'};
  auto old_control = msg.exchange_control(std::move(new_control));

  // Verify exchange occurred
  auto current_control = msg.control();
  EXPECT_EQ(current_control.size(), 7);
  EXPECT_EQ(current_control[0], 'N');
  EXPECT_EQ(current_control[3], 'C');

  // Verify old control returned
  EXPECT_EQ(old_control.size(), 4);
  EXPECT_EQ(old_control[0], 'I');
  EXPECT_EQ(old_control[3], 'T');
}

TEST_F(SocketMessageTest, ExchangeFlags) {
  socket_message msg;
  msg.set_flags(100);

  // Exchange flags
  int old_flags = msg.exchange_flags(200);

  // Verify exchange occurred
  EXPECT_EQ(msg.flags(), 200);
  EXPECT_EQ(old_flags, 100);

  // Test with zero flags
  int old_flags2 = msg.exchange_flags(0);
  EXPECT_EQ(msg.flags(), 0);
  EXPECT_EQ(old_flags2, 200);
}

TEST_F(SocketMessageTest, ExchangeEmptyBuffers) {
  socket_message msg;

  // Start with empty buffers
  std::vector<socket_buffer_type> empty_buffers;
  auto old_empty = msg.exchange_buffers(std::move(empty_buffers));
  EXPECT_TRUE(old_empty.empty());

  // Exchange empty for non-empty
  std::vector<socket_buffer_type> new_buffers;
  std::array<char, 25> data{};
  socket_buffer_type buf{data};
  new_buffers.push_back(buf);

  auto old_buffers = msg.exchange_buffers(std::move(new_buffers));
  EXPECT_TRUE(old_buffers.empty());
  EXPECT_EQ(msg.buffers().size(), 1);
}

TEST_F(SocketMessageTest, ExchangeEmptyControl) {
  socket_message msg;

  // Start with empty control
  std::vector<char> empty_control;
  auto old_empty = msg.exchange_control(std::move(empty_control));
  EXPECT_TRUE(old_empty.empty());

  // Exchange empty for non-empty
  std::vector<char> new_control{'D', 'A', 'T', 'A'};
  auto old_control = msg.exchange_control(std::move(new_control));
  EXPECT_TRUE(old_control.empty());
  EXPECT_EQ(msg.control().size(), 4);
}

// Test complete message scenarios
TEST_F(SocketMessageTest, CompleteMessageScenario) {
  socket_message msg;

  // Set up a complete message with all components
  socket_address addr;
  msg = addr;

  std::vector<socket_buffer_type> buffers;
  std::array<char, 256> data{};
  std::iota(data.begin(), data.end(), 0);
  socket_buffer_type buf{data};
  buffers.push_back(buf);
  msg.set_buffers(std::move(buffers));

  std::vector<char> control(64, 'C');
  msg.set_control(std::move(control));

  msg.set_flags(MSG_DONTWAIT | MSG_NOSIGNAL);

  // Verify all components
  EXPECT_EQ(msg.buffers().size(), 1);
  EXPECT_EQ(msg.buffers()[0].size(), 256);
  EXPECT_EQ(msg.control().size(), 64);
  EXPECT_EQ(msg.flags(), MSG_DONTWAIT | MSG_NOSIGNAL);

  // Verify data integrity
  auto retrieved_buffers = msg.buffers();
  EXPECT_EQ(retrieved_buffers[0][0], 0);
  EXPECT_EQ(retrieved_buffers[0][255], static_cast<char>(255));

  auto retrieved_control = msg.control();
  EXPECT_TRUE(
      std::all_of(retrieved_control.begin(), retrieved_control.end(),
                  [](char control_char) { return control_char == 'C'; }));
}

// Thread safety tests
TEST_F(SocketMessageTest, ThreadSafetyBasic) {
  socket_message msg;
  msg.set_flags(1000);

  std::atomic<int> reader_count{0};
  std::atomic<int> writer_count{0};
  constexpr int num_threads = 4;            // Reduced for simpler test
  constexpr int operations_per_thread = 10; // Reduced for simpler test

  std::vector<std::thread> threads;

  // Create reader threads
  for (int i = 0; i < num_threads / 2; ++i) {
    threads.emplace_back([&]() {
      for (int op = 0; op < operations_per_thread; ++op) {
        volatile int flags = msg.flags();
        (void)flags;
        auto buffers = msg.buffers();
        auto control = msg.control();
        auto address = msg.address();
        reader_count.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
    });
  }

  // Create writer threads
  for (int i = 0; i < num_threads / 2; ++i) {
    threads.emplace_back([&, i]() {
      for (int op = 0; op < operations_per_thread; ++op) {
        msg.set_flags((i + 1) * 100 + op);

        std::vector<char> control(op % 5 + 1, 'A' + (i % 26));
        msg.set_control(std::move(control));

        writer_count.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
    });
  }

  // Wait for completion
  for (auto &t : threads) {
    t.join();
  }

  // Verify operations completed
  EXPECT_EQ(reader_count.load(), (num_threads / 2) * operations_per_thread);
  EXPECT_EQ(writer_count.load(), (num_threads / 2) * operations_per_thread);

  // Message should be in a valid state
  auto final_flags = msg.flags();
  auto final_control = msg.control();
  EXPECT_GE(final_flags, 0);
  EXPECT_LE(final_control.size(), 6);
}

TEST_F(SocketMessageTest, ThreadSafeSwap) {
  socket_message msg1;
  socket_message msg2;

  msg1.set_flags(111);
  std::vector<char> control1{'1', '1', '1'};
  msg1.set_control(std::move(control1));

  msg2.set_flags(222);
  std::vector<char> control2{'2', '2', '2'};
  msg2.set_control(std::move(control2));

  std::atomic<int> swap_count{0};
  constexpr int num_swap_threads = 2; // Reduced for simpler test
  constexpr int swaps_per_thread = 5; // Reduced for simpler test

  std::vector<std::thread> threads;

  // Create swap threads
  for (int i = 0; i < num_swap_threads; ++i) {
    threads.emplace_back([&]() {
      for (int s = 0; s < swaps_per_thread; ++s) {
        swap(msg1, msg2);
        swap_count.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // Verify swaps completed
  EXPECT_EQ(swap_count.load(), num_swap_threads * swaps_per_thread);

  // Messages should still contain valid data (either original or swapped)
  auto final_flags1 = msg1.flags();
  auto final_flags2 = msg2.flags();
  auto final_control1 = msg1.control();
  auto final_control2 = msg2.control();

  EXPECT_TRUE((final_flags1 == 111 && final_flags2 == 222) ||
              (final_flags1 == 222 && final_flags2 == 111));
  EXPECT_EQ(final_control1.size(), 3);
  EXPECT_EQ(final_control2.size(), 3);
  EXPECT_TRUE((final_control1[0] == '1' && final_control2[0] == '2') ||
              (final_control1[0] == '2' && final_control2[0] == '1'));
}

// RAII and resource management tests
TEST_F(SocketMessageRAIITest, DestructorCleansUpProperly) {
  // Test that destructors don't crash with various data states
  { socket_message msg; }

  {
    socket_message msg;
    msg.set_flags(42);
  }

  {
    socket_message msg;
    std::vector<char> data(1000, 'X');
    msg.set_control(std::move(data));
  }

  {
    socket_message msg;
    std::vector<socket_buffer_type> buffers;
    std::array<char, 100> data{};
    socket_buffer_type buf{data};
    buffers.push_back(buf);
    msg.set_buffers(std::move(buffers));
  }

  // If we reach here, all destructors completed successfully
  SUCCEED();
}

TEST_F(SocketMessageRAIITest, MoveOperationsLeaveValidState) {
  socket_message msg1;
  msg1.set_flags(999);
  std::vector<char> control{'M', 'O', 'V', 'E'};
  msg1.set_control(std::move(control));

  socket_message msg2 = std::move(msg1);

  // msg1 should be in a valid state (though unspecified)
  // These operations should not crash
  [[maybe_unused]] auto flags1 = msg1.flags();
  auto moved_control1 = msg1.control();
  auto moved_buffers1 = msg1.buffers();
  auto moved_address1 = msg1.address();

  // msg1 should be assignable
  msg1.set_flags(777);
  EXPECT_EQ(msg1.flags(), 777);

  // msg2 should have the original data
  EXPECT_EQ(msg2.flags(), 999);
  auto control2 = msg2.control();
  EXPECT_EQ(control2.size(), 4);
  EXPECT_EQ(control2[0], 'M');
}
