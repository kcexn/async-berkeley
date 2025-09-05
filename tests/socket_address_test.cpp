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
#include "../src/io/socket/socket_address.hpp"

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

TEST_F(SocketAddressTest, TestPointerConstruction) {
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  socket_address address{reinterpret_cast<sockaddr_type *>(&addr),
                         sizeof(addr)};
  auto *data = std::ranges::data(address);
  EXPECT_EQ(std::memcmp(data, &addr, sizeof(addr)), 0);
}
