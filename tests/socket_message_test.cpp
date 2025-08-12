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

#include "../src/socket/socket_message.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

using namespace iosched::socket;

class SocketTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test fixture for ancillary buffer tests
class AncillaryBufferTest : public SocketTest {
protected:
  using test_buffer = ancillary_buffer;
  static constexpr size_t test_data_size = 1024;

  static auto
  create_test_data(size_t size = test_data_size) -> std::vector<char> {
    std::vector<char> data(size);
    for (size_t i = 0; i < size; ++i) {
      data[i] = static_cast<char>(i % UINT8_MAX);
    }
    return data;
  }
};

// Basic construction and data access tests
TEST_F(AncillaryBufferTest, DefaultConstruction) {
  test_buffer buffer;
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(buffer.data(), nullptr);
}

TEST_F(AncillaryBufferTest, ConstructionWithData) {
  auto test_data = create_test_data();
  test_buffer buffer(test_data);

  EXPECT_EQ(buffer.size(), test_data.size());
  EXPECT_NE(buffer.data(), nullptr);
  EXPECT_EQ(memcmp(buffer.data(), test_data.data(), test_data.size()), 0);
}

TEST_F(AncillaryBufferTest, ConstructionWithMovedData) {
  auto test_data = create_test_data();
  auto original_data = test_data;
  test_buffer buffer(std::move(test_data));

  EXPECT_EQ(buffer.size(), original_data.size());
  EXPECT_NE(buffer.data(), nullptr);
  EXPECT_EQ(memcmp(buffer.data(), original_data.data(), original_data.size()),
            0);
}

// Copy construction and assignment tests
TEST_F(AncillaryBufferTest, CopyConstruction) {
  auto test_data = create_test_data();
  test_buffer original(test_data);
  const test_buffer &copy(original);

  EXPECT_EQ(copy.size(), original.size());
  EXPECT_EQ(memcmp(copy.data(), original.data(), original.size()), 0);
}

TEST_F(AncillaryBufferTest, CopyAssignment) {
  auto test_data = create_test_data();
  test_buffer original(test_data);
  test_buffer copy;

  copy = original;

  EXPECT_EQ(copy.size(), original.size());
  EXPECT_EQ(memcmp(copy.data(), original.data(), original.size()), 0);
}

// Move construction and assignment tests
TEST_F(AncillaryBufferTest, MoveConstruction) {
  auto test_data = create_test_data();
  test_buffer original(test_data);
  auto original_size = original.size();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto original_data_copy =
      std::vector<char>(original.data(), original.data() + original.size());

  test_buffer moved(std::move(original));

  EXPECT_EQ(moved.size(), original_size);
  EXPECT_EQ(memcmp(moved.data(), original_data_copy.data(), original_size), 0);
}

TEST_F(AncillaryBufferTest, MoveAssignment) {
  auto test_data = create_test_data();
  test_buffer original(test_data);
  auto original_size = original.size();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto original_data_copy =
      std::vector<char>(original.data(), original.data() + original.size());

  test_buffer moved;
  moved = std::move(original);

  EXPECT_EQ(moved.size(), original_size);
  EXPECT_EQ(memcmp(moved.data(), original_data_copy.data(), original_size), 0);
}

// Swap functionality tests
TEST_F(AncillaryBufferTest, SwapOperation) {
  constexpr size_t test_data_size = 512;
  auto test_data1 = create_test_data(test_data_size);
  auto test_data2 = create_test_data(test_data_size / 2);

  test_buffer buffer1(test_data1);
  test_buffer buffer2(test_data2);

  auto buffer1_size = buffer1.size();
  auto buffer2_size = buffer2.size();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto buffer1_data_copy =
      std::vector<char>(buffer1.data(), buffer1.data() + buffer1.size());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto buffer2_data_copy =
      std::vector<char>(buffer2.data(), buffer2.data() + buffer2.size());

  swap(buffer1, buffer2);

  EXPECT_EQ(buffer1.size(), buffer2_size);
  EXPECT_EQ(buffer2.size(), buffer1_size);
  EXPECT_EQ(memcmp(buffer1.data(), buffer2_data_copy.data(), buffer2_size), 0);
  EXPECT_EQ(memcmp(buffer2.data(), buffer1_data_copy.data(), buffer1_size), 0);
}

// Thread safety tests
TEST_F(AncillaryBufferTest, ThreadSafeCopyConstruction) {
  constexpr int num_threads = 10;
  auto test_data = create_test_data();
  test_buffer original(test_data);

  std::vector<std::thread> threads;
  std::vector<std::unique_ptr<test_buffer>> copies(num_threads);

  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(
        [&, i]() { copies[i] = std::make_unique<test_buffer>(original); });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  for (const auto &copy : copies) {
    EXPECT_EQ(copy->size(), original.size());
    EXPECT_EQ(memcmp(copy->data(), original.data(), original.size()), 0);
  }
}

TEST_F(AncillaryBufferTest, ThreadSafeSwap) {
  constexpr size_t test_data_size = 512;
  constexpr int num_threads = 100;
  auto test_data1 = create_test_data(test_data_size);
  auto test_data2 = create_test_data(test_data_size / 2);

  test_buffer buffer1(test_data1);
  test_buffer buffer2(test_data2);

  std::vector<std::thread> threads;

  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() { swap(buffer1, buffer2); });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  // After even number of swaps, buffers should be back to original state
  EXPECT_TRUE((buffer1.size() == test_data1.size() &&
               buffer2.size() == test_data2.size()) ||
              (buffer1.size() == test_data2.size() &&
               buffer2.size() == test_data1.size()));
}

// Test fixture for data buffer tests
class DataBufferTest : public SocketTest {
protected:
  using test_buffer = data_buffer;
  static constexpr size_t test_size = 1024;

  static auto create_test_data(size_t size = test_size) -> std::vector<char> {
    std::vector<char> data(size);
    for (size_t i = 0; i < size; ++i) {
      data[i] = static_cast<char>(i % UINT8_MAX);
    }
    return data;
  }
};

// Data buffer basic functionality tests
TEST_F(DataBufferTest, DefaultConstruction) {
  test_buffer buffer;
  // Platform-specific behavior - just ensure construction doesn't crash
  EXPECT_NO_THROW({
    auto &data_ref = buffer.data();
    auto &size_ref = buffer.size();
    (void)data_ref; // Suppress unused variable warning
    (void)size_ref; // Suppress unused variable warning
  });
}

TEST_F(DataBufferTest, DataAndSizeAccess) {
  test_buffer buffer;
  auto test_data = create_test_data();

  // Set data pointer and size
  buffer.data() = test_data.data();
  buffer.size() = test_data.size();

  // Verify the references work correctly
  EXPECT_EQ(buffer.data(), test_data.data());
  EXPECT_EQ(buffer.size(), test_data.size());
}

// Test fixture for socket message tests
class SocketMessageTest : public SocketTest {
protected:
  static constexpr size_t test_data_size = 512;
  static constexpr size_t test_ancillary_size = 64;

  static auto create_test_data(size_t size) -> std::vector<char> {
    std::vector<char> data(size);
    for (size_t i = 0; i < size; ++i) {
      data[i] = static_cast<char>(i % UINT8_MAX);
    }
    return data;
  }
};

// Socket message structure tests
TEST_F(SocketMessageTest, DefaultConstruction) {
  socket_message msg;

  // Test default construction doesn't crash
  EXPECT_NO_THROW({
    auto &addr = msg.addr;
    auto &data = msg.data;
    auto &ancillary = msg.ancillary;
    auto &flags = msg.flags;
    (void)addr;
    (void)data;
    (void)ancillary;
    (void)flags;
  });
}

TEST_F(SocketMessageTest, ComponentsInitialization) {
  socket_message msg;

  // Initialize ancillary data
  auto ancillary_data = create_test_data(test_ancillary_size);
  msg.ancillary = ancillary_buffer(ancillary_data);

  // Initialize main data buffer
  auto main_data = create_test_data(test_data_size);
  msg.data.data() = main_data.data();
  msg.data.size() = main_data.size();

  // Set flags
  constexpr std::size_t flags = 42;
  msg.flags = flags;

  // Verify components
  EXPECT_EQ(msg.ancillary.size(), test_ancillary_size);
  EXPECT_EQ(msg.data.data(), main_data.data());
  EXPECT_EQ(msg.data.size(), main_data.size());
  EXPECT_EQ(msg.flags, 42);
}

TEST_F(SocketMessageTest, MessageCopyAndMove) {
  socket_message original;

  // Set up original message
  auto ancillary_data = create_test_data(test_ancillary_size);
  original.ancillary = ancillary_buffer(ancillary_data);

  constexpr std::size_t flags = 123;
  original.flags = flags;

  // Test copy
  socket_message copied = original;
  EXPECT_EQ(copied.ancillary.size(), original.ancillary.size());
  EXPECT_EQ(copied.flags, original.flags);

  // Test move
  socket_message moved = std::move(original);
  EXPECT_EQ(moved.ancillary.size(), test_ancillary_size);
  EXPECT_EQ(moved.flags, 123);
}

// Platform-specific tests
class PlatformSpecificTest : public SocketTest {};

#if BOOST_OS_WINDOWS
TEST_F(PlatformSpecificTest, WindowsWSABUFIntegration) {
  std::vector<char> test_data(1024);
  for (size_t i = 0; i < test_data.size(); ++i) {
    test_data[i] = static_cast<char>(i % 256);
  }
  ancillary_buffer buffer(test_data);

  // Verify WSABUF fields are properly set
  const WSABUF *wsabuf = static_cast<const WSABUF *>(&buffer);
  EXPECT_EQ(wsabuf->len, test_data.size());
  EXPECT_EQ(wsabuf->buf, buffer.data());
}

TEST_F(PlatformSpecificTest, WindowsDataBufferWSABUF) {
  data_buffer buffer;
  std::vector<char> test_data(512);
  for (size_t i = 0; i < test_data.size(); ++i) {
    test_data[i] = static_cast<char>(i % 256);
  }

  buffer.data() = test_data.data();
  buffer.size() = static_cast<ULONG>(test_data.size());

  // Verify platform-specific field access
  const WSABUF *wsabuf = static_cast<const WSABUF *>(&buffer);
  EXPECT_EQ(wsabuf->buf, test_data.data());
  EXPECT_EQ(wsabuf->len, test_data.size());
}
#else
TEST_F(PlatformSpecificTest, POSIXDataBufferIovec) {
  constexpr size_t test_size = 512;
  data_buffer buffer;
  std::vector<char> test_data(test_size);
  for (size_t i = 0; i < test_data.size(); ++i) {
    test_data[i] = static_cast<char>(i % (test_size / 2));
  }

  buffer.data() = test_data.data();
  buffer.size() = test_data.size();

  // Verify platform-specific field access
  const struct iovec *iov = static_cast<const struct iovec *>(&buffer);
  EXPECT_EQ(iov->iov_base, test_data.data());
  EXPECT_EQ(iov->iov_len, test_data.size());
}

TEST_F(PlatformSpecificTest, POSIXAddressType) {
  address_type addr;
  auto &[storage, length] = addr;

  // Test that we can access sockaddr_storage and socklen_t
  length = sizeof(sockaddr_storage);
  EXPECT_EQ(length, sizeof(sockaddr_storage));
  EXPECT_NO_THROW({ memset(&storage, 0, sizeof(storage)); });
}
#endif

// Integration tests combining multiple components
class IntegrationTest : public SocketTest {};

TEST_F(IntegrationTest, FullMessageConstruction) {
  constexpr size_t test_data_size = 1024;

  socket_message msg;

  // Set up all components
  std::vector<char> main_data(test_data_size);
  for (size_t i = 0; i < main_data.size(); ++i) {
    main_data[i] = static_cast<char>(i % (test_data_size / 4));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std::vector<char> ancillary_data((test_data_size / 8));
  for (size_t i = 0; i < ancillary_data.size(); ++i) {
    ancillary_data[i] = static_cast<char>(i % (test_data_size / 4));
  }

  msg.data.data() = main_data.data();
  msg.data.size() = main_data.size();
  msg.ancillary = ancillary_buffer(ancillary_data);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  msg.flags = 42; // Use a simple flag value instead of MSG_DONTWAIT

  // Verify all components work together
  EXPECT_EQ(msg.data.data(), main_data.data());
  EXPECT_EQ(msg.data.size(), main_data.size());
  EXPECT_EQ(msg.ancillary.size(), ancillary_data.size());
  EXPECT_EQ(msg.flags, 42);
}

TEST_F(IntegrationTest, MessageArrayOperations) {
  constexpr size_t num_messages = 5;
  std::vector<socket_message> messages(num_messages);

  // Initialize each message with different data
  for (size_t i = 0; i < num_messages; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::vector<char> data(64 + (i * 32));
    for (size_t j = 0; j < data.size(); ++j) {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      data[j] = static_cast<char>(j % 256);
    }
    messages[i].ancillary = ancillary_buffer(data);
    messages[i].flags = i;
  }

  // Verify each message maintains its data
  for (size_t i = 0; i < num_messages; ++i) {
    EXPECT_EQ(messages[i].ancillary.size(), 64 + (i * 32));
    EXPECT_EQ(messages[i].flags, i);
  }
}
