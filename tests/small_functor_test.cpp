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

  small_functor<void(), sizeof(T)> h{std::move(g)};
  EXPECT_TRUE(h);
  EXPECT_FALSE(g);
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

TEST_F(SmallFunctorTest, SwapDestructorTest)
{
  using std::swap;

  // Track construction and destruction counts
  static int construct_count = 0;
  static int destruct_count = 0;

  struct TrackedCallable {
    TrackedCallable() { ++construct_count; }
    TrackedCallable(const TrackedCallable &) { ++construct_count; }
    TrackedCallable(TrackedCallable &&) noexcept { ++construct_count; }
    ~TrackedCallable() { ++destruct_count; }
    void operator()() const {}
  };

  construct_count = 0;
  destruct_count = 0;

  {
    using T = TrackedCallable;
    auto fn = small_functor<void(), sizeof(T)>(TrackedCallable{});
    auto g = small_functor<void(), sizeof(T)>(TrackedCallable{});

    ASSERT_TRUE(fn);
    ASSERT_TRUE(g);

    // After construction, we should have 2 constructions (temporary + placement
    // new for each) and destructions of the temporaries
    int constructs_before_swap = construct_count;
    int destructs_before_swap = destruct_count;

    // Perform swap - this should not leak the temporary
    swap(fn, g);

    EXPECT_EQ(construct_count, constructs_before_swap + 3);
    EXPECT_EQ(destruct_count, destructs_before_swap + 3);

    EXPECT_TRUE(fn);
    EXPECT_TRUE(g);
  }

  // After scope exit, both functors should be destroyed
  // We should have 2 more destructions (one for each functor)
  EXPECT_EQ(construct_count, destruct_count);
}
// NOLINTEND
