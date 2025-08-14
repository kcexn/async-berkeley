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
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace iosched::socket;

class SocketAddressTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a test IPv4 address
    std::memset(&ipv4_addr_, 0, sizeof(ipv4_addr_));
    ipv4_addr_.sin_family = AF_INET;
    ipv4_addr_.sin_port = htons(8080);
    ipv4_addr_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ipv4_size_ = sizeof(ipv4_addr_);

    // Create a test IPv6 address
    std::memset(&ipv6_addr_, 0, sizeof(ipv6_addr_));
    ipv6_addr_.sin6_family = AF_INET6;
    ipv6_addr_.sin6_port = htons(9090);
    ipv6_addr_.sin6_addr = in6addr_loopback;
    ipv6_size_ = sizeof(ipv6_addr_);
  }

  void TearDown() override {}

  sockaddr_in ipv4_addr_{};
  socklen_t ipv4_size_{};
  sockaddr_in6 ipv6_addr_{};
  socklen_t ipv6_size_{};
};

TEST_F(SocketAddressTest, ParameterizedConstruction) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                      ipv4_size_);

  EXPECT_NE(addr.data(), nullptr);
  EXPECT_NE(addr.size(), nullptr);
  EXPECT_EQ(*addr.size(), ipv4_size_);

  // Verify the data was copied correctly
  const auto *stored_addr = reinterpret_cast<const sockaddr_in *>(addr.data());
  EXPECT_EQ(stored_addr->sin_family, AF_INET);
  EXPECT_EQ(stored_addr->sin_port, htons(8080));
  EXPECT_EQ(stored_addr->sin_addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST_F(SocketAddressTest, ParameterizedConstructionIPv6) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv6_addr_),
                      ipv6_size_);

  EXPECT_NE(addr.data(), nullptr);
  EXPECT_NE(addr.size(), nullptr);
  EXPECT_EQ(*addr.size(), ipv6_size_);

  // Verify the data was copied correctly
  const auto *stored_addr = reinterpret_cast<const sockaddr_in6 *>(addr.data());
  EXPECT_EQ(stored_addr->sin6_family, AF_INET6);
  EXPECT_EQ(stored_addr->sin6_port, htons(9090));
  EXPECT_EQ(std::memcmp(&stored_addr->sin6_addr, &in6addr_loopback,
                        sizeof(in6addr_loopback)),
            0);
}

TEST_F(SocketAddressTest, CopyConstruction) {
  socket_address original(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                          ipv4_size_);
  socket_address copy(original);

  // Verify both objects have the same data but different addresses
  EXPECT_NE(copy.data(), original.data());
  EXPECT_NE(copy.size(), original.size());
  EXPECT_EQ(*copy.size(), *original.size());

  // Verify the address data is the same
  const auto *original_addr =
      reinterpret_cast<const sockaddr_in *>(original.data());
  const auto *copy_addr = reinterpret_cast<const sockaddr_in *>(copy.data());

  EXPECT_EQ(copy_addr->sin_family, original_addr->sin_family);
  EXPECT_EQ(copy_addr->sin_port, original_addr->sin_port);
  EXPECT_EQ(copy_addr->sin_addr.s_addr, original_addr->sin_addr.s_addr);
}

TEST_F(SocketAddressTest, MoveConstruction) {
  socket_address original(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                          ipv4_size_);
  socklen_t original_size = *original.size();

  socket_address moved(std::move(original));

  // Verify moved object has the expected data
  EXPECT_NE(moved.data(), nullptr);
  EXPECT_NE(moved.size(), nullptr);
  EXPECT_EQ(*moved.size(), original_size);

  const auto *moved_addr = reinterpret_cast<const sockaddr_in *>(moved.data());
  EXPECT_EQ(moved_addr->sin_family, AF_INET);
  EXPECT_EQ(moved_addr->sin_port, htons(8080));
  EXPECT_EQ(moved_addr->sin_addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST_F(SocketAddressTest, CopyAssignment) {
  socket_address original(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                          ipv4_size_);
  socket_address assigned(reinterpret_cast<const sockaddr *>(&ipv6_addr_),
                          ipv6_size_);

  // Verify they start different
  EXPECT_NE(*assigned.size(), *original.size());

  assigned = original;

  // Verify assignment worked
  EXPECT_NE(assigned.data(), original.data());
  EXPECT_NE(assigned.size(), original.size());
  EXPECT_EQ(*assigned.size(), *original.size());

  const auto *original_addr =
      reinterpret_cast<const sockaddr_in *>(original.data());
  const auto *assigned_addr =
      reinterpret_cast<const sockaddr_in *>(assigned.data());

  EXPECT_EQ(assigned_addr->sin_family, original_addr->sin_family);
  EXPECT_EQ(assigned_addr->sin_port, original_addr->sin_port);
  EXPECT_EQ(assigned_addr->sin_addr.s_addr, original_addr->sin_addr.s_addr);
}

TEST_F(SocketAddressTest, MoveAssignment) {
  socket_address original(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                          ipv4_size_);
  socket_address assigned(reinterpret_cast<const sockaddr *>(&ipv6_addr_),
                          ipv6_size_);
  socklen_t original_size = *original.size();

  assigned = std::move(original);

  // Verify assignment worked
  EXPECT_NE(assigned.data(), nullptr);
  EXPECT_NE(assigned.size(), nullptr);
  EXPECT_EQ(*assigned.size(), original_size);

  const auto *assigned_addr =
      reinterpret_cast<const sockaddr_in *>(assigned.data());
  EXPECT_EQ(assigned_addr->sin_family, AF_INET);
  EXPECT_EQ(assigned_addr->sin_port, htons(8080));
  EXPECT_EQ(assigned_addr->sin_addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST_F(SocketAddressTest, SelfAssignment) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                      ipv4_size_);

  // Self-assignment should be safe
  addr = addr;

  EXPECT_NE(addr.data(), nullptr);
  EXPECT_NE(addr.size(), nullptr);
  EXPECT_EQ(*addr.size(), ipv4_size_);

  const auto *stored_addr = reinterpret_cast<const sockaddr_in *>(addr.data());
  EXPECT_EQ(stored_addr->sin_family, AF_INET);
  EXPECT_EQ(stored_addr->sin_port, htons(8080));
  EXPECT_EQ(stored_addr->sin_addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST_F(SocketAddressTest, DataAccessMutable) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                      ipv4_size_);

  auto *data_ptr = addr.data();
  EXPECT_NE(data_ptr, nullptr);

  // Modify through mutable pointer
  auto *ipv4_ptr = reinterpret_cast<sockaddr_in *>(data_ptr);
  ipv4_ptr->sin_port = htons(7777);

  // Verify the change
  const auto *const_data = addr.data();
  const auto *const_ipv4 = reinterpret_cast<const sockaddr_in *>(const_data);
  EXPECT_EQ(const_ipv4->sin_port, htons(7777));
}

TEST_F(SocketAddressTest, DataAccessConst) {
  const socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                            ipv4_size_);

  const auto *data_ptr = addr.data();
  EXPECT_NE(data_ptr, nullptr);

  const auto *ipv4_ptr = reinterpret_cast<const sockaddr_in *>(data_ptr);
  EXPECT_EQ(ipv4_ptr->sin_family, AF_INET);
  EXPECT_EQ(ipv4_ptr->sin_port, htons(8080));
  EXPECT_EQ(ipv4_ptr->sin_addr.s_addr, htonl(INADDR_LOOPBACK));
}

TEST_F(SocketAddressTest, SizeAccessMutable) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                      ipv4_size_);

  auto *size_ptr = addr.size();
  EXPECT_NE(size_ptr, nullptr);
  EXPECT_EQ(*size_ptr, ipv4_size_);

  // Modify through mutable pointer
  *size_ptr = 42;
  EXPECT_EQ(*addr.size(), 42);
}

TEST_F(SocketAddressTest, SizeAccessConst) {
  const socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                            ipv4_size_);

  const auto *size_ptr = addr.size();
  EXPECT_NE(size_ptr, nullptr);
  EXPECT_EQ(*size_ptr, ipv4_size_);
}

TEST_F(SocketAddressTest, ZeroSizeConstruction) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_), 0);

  EXPECT_NE(addr.data(), nullptr);
  EXPECT_NE(addr.size(), nullptr);
  EXPECT_EQ(*addr.size(), 0);
}

TEST_F(SocketAddressTest, EqualityOperator) {
  socket_address addr1(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                       ipv4_size_);
  socket_address addr2(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                       ipv4_size_);

  EXPECT_TRUE(addr1 == addr2);
  EXPECT_TRUE(addr2 == addr1);
}

TEST_F(SocketAddressTest, EqualityOperatorSelfComparison) {
  socket_address addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                      ipv4_size_);

  EXPECT_TRUE(addr == addr);
}

TEST_F(SocketAddressTest, InequalityOperatorDifferentFamily) {
  socket_address ipv4_addr(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                           ipv4_size_);
  socket_address ipv6_addr(reinterpret_cast<const sockaddr *>(&ipv6_addr_),
                           ipv6_size_);

  EXPECT_FALSE(ipv4_addr == ipv6_addr);
  EXPECT_FALSE(ipv6_addr == ipv4_addr);
  EXPECT_TRUE(ipv4_addr != ipv6_addr);
  EXPECT_TRUE(ipv6_addr != ipv4_addr);
}

TEST_F(SocketAddressTest, InequalityOperatorDifferentPort) {
  sockaddr_in different_port = ipv4_addr_;
  different_port.sin_port = htons(9999);

  socket_address addr1(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                       ipv4_size_);
  socket_address addr2(reinterpret_cast<const sockaddr *>(&different_port),
                       ipv4_size_);

  EXPECT_FALSE(addr1 == addr2);
  EXPECT_FALSE(addr2 == addr1);
  EXPECT_TRUE(addr1 != addr2);
  EXPECT_TRUE(addr2 != addr1);
}

TEST_F(SocketAddressTest, InequalityOperatorDifferentSize) {
  socket_address addr1(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                       ipv4_size_);
  socket_address addr2(reinterpret_cast<const sockaddr *>(&ipv4_addr_),
                       ipv4_size_ - 1);

  EXPECT_FALSE(addr1 == addr2);
  EXPECT_FALSE(addr2 == addr1);
  EXPECT_TRUE(addr1 != addr2);
  EXPECT_TRUE(addr2 != addr1);
}

TEST_F(SocketAddressTest, EqualityOperatorZeroSize) {
  socket_address addr1(reinterpret_cast<const sockaddr *>(&ipv4_addr_), 0);
  socket_address addr2(reinterpret_cast<const sockaddr *>(&ipv6_addr_), 0);

  EXPECT_TRUE(addr1 == addr2);
  EXPECT_TRUE(addr2 == addr1);
}