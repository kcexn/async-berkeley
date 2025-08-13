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

#include "../src/buffers/socket_buffer.hpp"
#include "../src/socket/socket_handle.hpp"
#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

using namespace iosched::buffers;
using namespace iosched::socket;

class SocketBufferTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}

  static auto create_test_socket_message() -> socket_message {
    socket_message msg;
    msg.flags = 42;
    return msg;
  }

  static auto create_test_socket() -> iosched::socket::socket_handle {
    return iosched::socket::socket_handle{123};
  }
};

class SocketBufferBaseTest : public SocketBufferTest {
protected:
  static constexpr auto test_socket_value = 456;

  class TestableSocketBufferBase : public socket_buffer_base {
  public:
    using socket_buffer_base::read_buffer;
    using socket_buffer_base::socket;
    using socket_buffer_base::socket_buffer_base;
    using socket_buffer_base::write_buffer;
  };
};

TEST_F(SocketBufferBaseTest, DefaultConstruction) {
  TestableSocketBufferBase base;

  EXPECT_FALSE(base.socket);
  EXPECT_TRUE(base.read_buffer.empty());
  EXPECT_TRUE(base.write_buffer.empty());
}

TEST_F(SocketBufferBaseTest, ConstructionWithParameters) {
  auto test_socket = create_test_socket();
  socket_buffer_base::buffer_type read_buf;
  socket_buffer_base::buffer_type write_buf;

  auto msg1 = std::make_shared<socket_message>(create_test_socket_message());
  auto msg2 = std::make_shared<socket_message>(create_test_socket_message());

  read_buf.push(msg1);
  write_buf.push(msg2);

  TestableSocketBufferBase base(std::move(test_socket), std::move(read_buf),
                                std::move(write_buf));

  EXPECT_TRUE(base.socket);
  EXPECT_FALSE(base.read_buffer.empty());
  EXPECT_FALSE(base.write_buffer.empty());
  EXPECT_EQ(base.read_buffer.size(), 1);
  EXPECT_EQ(base.write_buffer.size(), 1);
}

// Copy construction is deleted for socket_buffer_base

TEST_F(SocketBufferBaseTest, MoveConstruction) {
  auto test_socket = create_test_socket();
  TestableSocketBufferBase original(std::move(test_socket));

  auto msg = std::make_shared<socket_message>(create_test_socket_message());
  original.read_buffer.push(msg);

  TestableSocketBufferBase moved(std::move(original));

  EXPECT_TRUE(moved.socket);
  EXPECT_FALSE(moved.read_buffer.empty());
}

// Copy assignment is deleted for socket_buffer_base

TEST_F(SocketBufferBaseTest, MoveAssignment) {
  auto test_socket1 = create_test_socket();
  auto test_socket2 = iosched::socket::socket_handle{789};

  TestableSocketBufferBase original(std::move(test_socket1));
  TestableSocketBufferBase moved(std::move(test_socket2));

  auto msg = std::make_shared<socket_message>(create_test_socket_message());
  original.read_buffer.push(msg);

  moved = std::move(original);

  EXPECT_TRUE(moved.socket);
  EXPECT_FALSE(moved.read_buffer.empty());
}

TEST_F(SocketBufferBaseTest, SelfMoveAssignment) {
  auto test_socket = create_test_socket();
  TestableSocketBufferBase buffer(std::move(test_socket));

  auto msg = std::make_shared<socket_message>(create_test_socket_message());
  buffer.read_buffer.push(msg);

  bool original_socket_valid = static_cast<bool>(buffer.socket);
  auto original_read_size = buffer.read_buffer.size();
  auto original_write_size = buffer.write_buffer.size();

  buffer = std::move(buffer);

  EXPECT_EQ(static_cast<bool>(buffer.socket), original_socket_valid);
  EXPECT_EQ(buffer.read_buffer.size(), original_read_size);
  EXPECT_EQ(buffer.write_buffer.size(), original_write_size);
}

TEST_F(SocketBufferBaseTest, SwapOperation) {
  auto test_socket1 = iosched::socket::socket_handle{111};
  auto test_socket2 = iosched::socket::socket_handle{222};

  TestableSocketBufferBase buf1(std::move(test_socket1));
  TestableSocketBufferBase buf2(std::move(test_socket2));

  auto msg1 = std::make_shared<socket_message>(create_test_socket_message());
  auto msg2 = std::make_shared<socket_message>(create_test_socket_message());

  buf1.read_buffer.push(msg1);
  buf2.write_buffer.push(msg2);

  swap(buf1, buf2);

  EXPECT_TRUE(buf1.socket);
  EXPECT_TRUE(buf2.socket);
  EXPECT_TRUE(buf1.read_buffer.empty());
  EXPECT_FALSE(buf1.write_buffer.empty());
  EXPECT_FALSE(buf2.read_buffer.empty());
  EXPECT_TRUE(buf2.write_buffer.empty());
}

// ThreadSafeCopyConstruction test removed since copy construction is deleted

TEST_F(SocketBufferBaseTest, ThreadSafeSwap) {
  constexpr int num_threads = 50;
  auto test_socket1 = iosched::socket::socket_handle{333};
  auto test_socket2 = iosched::socket::socket_handle{444};

  TestableSocketBufferBase buf1(std::move(test_socket1));
  TestableSocketBufferBase buf2(std::move(test_socket2));

  std::vector<std::thread> threads;

  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() { swap(buf1, buf2); });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  // Both sockets should still be valid after the swaps
  EXPECT_TRUE(buf1.socket);
  EXPECT_TRUE(buf2.socket);
}

class SocketReadBufferTest : public SocketBufferTest {
protected:
  class TestableSocketReadBuffer : public socket_read_buffer {
  public:
    using socket_buffer_base::read_buffer;
    using socket_buffer_base::socket;
    using socket_buffer_base::write_buffer;
  };
};

TEST_F(SocketReadBufferTest, ReadFromEmptyBuffer) {
  socket_read_buffer reader;

  auto result = reader.read();

  EXPECT_EQ(result, nullptr);
}

TEST_F(SocketReadBufferTest, ReadSingleMessage) {
  TestableSocketReadBuffer reader;

  auto msg = std::make_shared<socket_message>(create_test_socket_message());
  reader.read_buffer.push(msg);

  auto result = reader.read();

  EXPECT_NE(result, nullptr);
  EXPECT_EQ(result->flags, 42);
  EXPECT_TRUE(reader.read_buffer.empty());
}

TEST_F(SocketReadBufferTest, ReadMultipleMessages) {
  TestableSocketReadBuffer reader;

  constexpr int num_messages = 5;
  for (int i = 0; i < num_messages; ++i) {
    auto msg = std::make_shared<socket_message>(create_test_socket_message());
    msg->flags = i;
    reader.read_buffer.push(msg);
  }

  for (int i = 0; i < num_messages; ++i) {
    auto result = reader.read();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->flags, i);
  }

  EXPECT_TRUE(reader.read_buffer.empty());
  EXPECT_EQ(reader.read(), nullptr);
}

TEST_F(SocketReadBufferTest, ThreadSafeRead) {
  constexpr int num_threads = 10;
  constexpr int messages_per_thread = 5;

  TestableSocketReadBuffer reader;

  for (int i = 0; i < num_threads * messages_per_thread; ++i) {
    auto msg = std::make_shared<socket_message>(create_test_socket_message());
    msg->flags = i;
    reader.read_buffer.push(msg);
  }

  std::vector<std::thread> threads;
  std::vector<std::vector<std::shared_ptr<socket_message>>> results(
      num_threads);

  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i]() {
      for (int j = 0; j < messages_per_thread; ++j) {
        auto result = reader.read();
        if (result) {
          results[i].push_back(result);
        }
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  int total_messages = 0;
  for (const auto &thread_results : results) {
    total_messages += thread_results.size();
  }

  EXPECT_EQ(total_messages, num_threads * messages_per_thread);
  EXPECT_TRUE(reader.read_buffer.empty());
}

class SocketWriteBufferTest : public SocketBufferTest {
protected:
  class TestableSocketWriteBuffer : public socket_write_buffer {
  public:
    using socket_buffer_base::read_buffer;
    using socket_buffer_base::socket;
    using socket_buffer_base::write_buffer;
  };
};

TEST_F(SocketWriteBufferTest, WriteSingleMessage) {
  TestableSocketWriteBuffer writer;

  auto msg = create_test_socket_message();
  writer.write(std::move(msg));

  EXPECT_FALSE(writer.write_buffer.empty());
  EXPECT_EQ(writer.write_buffer.size(), 1);

  auto stored_msg = writer.write_buffer.front();
  EXPECT_NE(stored_msg, nullptr);
  EXPECT_EQ(stored_msg->flags, 42);
}

TEST_F(SocketWriteBufferTest, WriteMultipleMessages) {
  TestableSocketWriteBuffer writer;

  constexpr int num_messages = 10;
  for (int i = 0; i < num_messages; ++i) {
    auto msg = create_test_socket_message();
    msg.flags = i;
    writer.write(std::move(msg));
  }

  EXPECT_EQ(writer.write_buffer.size(), num_messages);

  for (int i = 0; i < num_messages; ++i) {
    auto stored_msg = writer.write_buffer.front();
    writer.write_buffer.pop();
    EXPECT_NE(stored_msg, nullptr);
    EXPECT_EQ(stored_msg->flags, i);
  }
}

TEST_F(SocketWriteBufferTest, ThreadSafeWrite) {
  constexpr int num_threads = 10;
  constexpr int messages_per_thread = 5;

  TestableSocketWriteBuffer writer;

  std::vector<std::thread> threads;

  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i]() {
      for (int j = 0; j < messages_per_thread; ++j) {
        auto msg = create_test_socket_message();
        msg.flags = i * messages_per_thread + j;
        writer.write(std::move(msg));
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(writer.write_buffer.size(), num_threads * messages_per_thread);

  std::vector<int> flags_found;
  flags_found.reserve(num_threads * messages_per_thread);

  while (!writer.write_buffer.empty()) {
    auto msg = writer.write_buffer.front();
    writer.write_buffer.pop();
    flags_found.push_back(msg->flags);
  }

  std::ranges::sort(flags_found);
  for (int i = 0; i < num_threads * messages_per_thread; ++i) {
    EXPECT_EQ(flags_found[i], i);
  }
}

class SocketBufferIntegrationTest : public SocketBufferTest {
protected:
  class TestableSocketBuffer : public socket_buffer {
  public:
    TestableSocketBuffer() = default;
    TestableSocketBuffer(socket_buffer_base::socket_handle_type sock)
        : socket_buffer_base(std::move(sock)) {}
    using socket_buffer_base::read_buffer;
    using socket_buffer_base::socket;
    using socket_buffer_base::write_buffer;

    auto has_write_messages() -> bool {
      std::lock_guard lock{mtx};
      return !write_buffer.empty();
    }

    auto move_write_to_read() -> bool {
      std::lock_guard lock{mtx};
      if (write_buffer.empty()) {
        return false;
      }
      auto msg = write_buffer.front();
      write_buffer.pop();
      read_buffer.push(msg);
      return true;
    }

    auto get_write_buffer_size() -> size_t {
      std::lock_guard lock{mtx};
      return write_buffer.size();
    }

    auto get_read_buffer_size() -> size_t {
      std::lock_guard lock{mtx};
      return read_buffer.size();
    }

  private:
    using socket_buffer_base::mtx;
  };
};

TEST_F(SocketBufferIntegrationTest, FullDuplexOperation) {
  auto test_socket = create_test_socket();
  TestableSocketBuffer buffer(std::move(test_socket));

  auto write_msg = create_test_socket_message();
  write_msg.flags = 100;
  buffer.write(std::move(write_msg));

  auto read_msg =
      std::make_shared<socket_message>(create_test_socket_message());
  read_msg->flags = 200;
  buffer.read_buffer.push(read_msg);

  EXPECT_TRUE(buffer.socket);
  EXPECT_FALSE(buffer.write_buffer.empty());
  EXPECT_FALSE(buffer.read_buffer.empty());

  auto result = buffer.read();
  EXPECT_NE(result, nullptr);
  EXPECT_EQ(result->flags, 200);
  EXPECT_TRUE(buffer.read_buffer.empty());

  EXPECT_FALSE(buffer.write_buffer.empty());
  auto written_msg = buffer.write_buffer.front();
  EXPECT_EQ(written_msg->flags, 100);
}

TEST_F(SocketBufferIntegrationTest, ConcurrentReadWrite) {
  constexpr int num_read_threads = 5;
  constexpr int num_write_threads = 5;
  constexpr int operations_per_thread = 10;

  TestableSocketBuffer buffer;

  std::vector<std::thread> threads;
  std::atomic<int> messages_read{0};

  threads.reserve(num_read_threads + num_write_threads);

  for (int i = 0; i < num_write_threads; ++i) {
    threads.emplace_back([&, i]() {
      for (int j = 0; j < operations_per_thread; ++j) {
        auto msg = create_test_socket_message();
        msg.flags = i * operations_per_thread + j;
        buffer.write(std::move(msg));
      }
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  for (int i = 0; i < num_read_threads; ++i) {
    threads.emplace_back([&]() {
      while (messages_read.load() < num_write_threads * operations_per_thread) {
        buffer.move_write_to_read();

        auto result = buffer.read();
        if (result) {
          messages_read.fetch_add(1);
        }

        std::this_thread::yield();
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  EXPECT_EQ(messages_read.load(), num_write_threads * operations_per_thread);
}

TEST_F(SocketBufferIntegrationTest, VirtualInheritanceCorrectness) {
  TestableSocketBuffer buffer;

  socket_read_buffer *read_ptr = &buffer;
  socket_write_buffer *write_ptr = &buffer;
  socket_buffer_base *base_ptr = &buffer;

  EXPECT_EQ(&buffer.socket, &buffer.socket);
  EXPECT_EQ(&buffer.read_buffer, &buffer.read_buffer);
  EXPECT_EQ(&buffer.write_buffer, &buffer.write_buffer);
}
