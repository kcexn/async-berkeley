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
// NOLINTBEGIN
#include "io/socket/socket_address.hpp"

#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace io::socket;

class SocketAddressTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(SocketAddressTest, TestPointerConstruction)
{
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  socket_address<sockaddr_in> address1{reinterpret_cast<sockaddr_type *>(&addr),
                                       sizeof(addr)};
  auto *data = std::ranges::data(address1);
  EXPECT_EQ(std::memcmp(data, &addr, sizeof(addr)), 0);

  auto address2 = make_address(&addr);
  EXPECT_EQ(address1, address2);
}

TEST_F(SocketAddressTest, TestOrdering)
{
  struct sockaddr_in addrv4 {};
  addrv4.sin_family = AF_INET;

  auto addr1 = socket_address<sockaddr_in>(&addrv4);
  addr1->sin_port = 8080;
  auto addr2 = socket_address<sockaddr_in>(&addrv4);
  addr2->sin_port = 8081;

  EXPECT_LT(addr1, addr2);

  struct sockaddr_in6 addrv6 {};
  addrv6.sin6_family = AF_INET6;
  auto addr3 = socket_address<sockaddr_in6>(&addrv6);
  addr3->sin6_port = 8079;

  auto spanv4 = std::span(std::ranges::data(addr1), std::ranges::size(addr1));
  EXPECT_LT(spanv4, addr3);
}
// NOLINTEND
