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
#include "io/socket/socket_option.hpp"

#include <gtest/gtest.h>

using namespace io::socket;

class SocketOptionTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SocketOptionTest, TestSizeConstructor) {
  socket_option<int> option1{sizeof(short)};
  EXPECT_EQ(std::ranges::size(option1), sizeof(short));
}

TEST_F(SocketOptionTest, TestSpanConstructor) {
  socket_option<int> option1{};
  EXPECT_EQ(*option1, 0);
  int tmp = 1;
  std::span<const std::byte, sizeof(int)> span_{
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const std::byte *>(&tmp), sizeof(int)};
  socket_option<int> option2{span_};
  EXPECT_EQ(*option2, 1);
  option1 = span_;
  EXPECT_EQ(option1, option2);
}

TEST_F(SocketOptionTest, TestValueConstructor) {
  socket_option<int> option1{1};
  EXPECT_EQ(*option1, 1);
}
