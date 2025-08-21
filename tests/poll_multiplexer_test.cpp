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

#include "../src/execution/context.hpp"
#include "../src/execution/poll_multiplexer.hpp"
#include <gtest/gtest.h>


using namespace io::execution;

class PollMultiplexerTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PollMultiplexerTest, IsMultiplexerTest) {
  using interval_type = poll_multiplexer::interval_type;
  context<poll_multiplexer> ctx{};
  ctx.wait(interval_type{0});
}
