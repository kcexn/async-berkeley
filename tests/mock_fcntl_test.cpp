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
#include "io/io.hpp"

#include <exec/async_scope.hpp>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

static bool exception_test = false;
int fcntl(int fd, int __cmd, ...)
{
  if (exception_test)
  {
    errno = ENOMEM;
    return -1;
  }
  return 0;
}

class MockFcntlTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MockFcntlTest, TestPushSocket)
{
  exception_test = true;
  basic_triggers<poll_multiplexer> poller;
  EXPECT_THROW(poller.emplace(AF_INET, SOCK_STREAM, 0), std::system_error);
}
// NOLINTEND
