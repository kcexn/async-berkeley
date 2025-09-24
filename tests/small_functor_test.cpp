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
#include "io/detail/small_functor.hpp"

#include <gtest/gtest.h>

using namespace io::detail;

class SmallFunctorTest : public ::testing::TestWithParam<bool> {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SmallFunctorTest, CopyTest)
{
  using std::swap;
  using T = decltype([]() {});
  auto fn = small_functor<void(), sizeof(T)>([]() {});
  ASSERT_TRUE(fn);
  auto g = small_functor<void(), sizeof(T)>();
  ASSERT_FALSE(g);

  fn = fn;
  ASSERT_TRUE(fn);

  g = fn;
  EXPECT_TRUE(g);
}

TEST_F(SmallFunctorTest, MoveTest)
{
  using std::swap;
  auto move_only = std::make_unique<int>(0);
  using T = decltype([tmp = std::make_unique<int>()]() {});
  auto fn = small_functor<void(), sizeof(T)>([tmp = std::move(move_only)]() {});
  ASSERT_TRUE(fn);

  auto g = small_functor<void(), sizeof(T)>();
  ASSERT_FALSE(g);

  g = std::move(fn);
  EXPECT_TRUE(g);
  EXPECT_FALSE(fn);
}

TEST_F(SmallFunctorTest, SwapTest)
{
  using std::swap;
  using T = decltype([]() {});
  auto fn = small_functor<void(), sizeof(T)>([]() {});
  ASSERT_TRUE(fn);
  auto g = fn;
  ASSERT_TRUE(g);

  swap(fn, fn);
  EXPECT_TRUE(fn);

  fn = g;
  EXPECT_TRUE(g);
}
// NOLINTEND
