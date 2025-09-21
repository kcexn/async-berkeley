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
#include "io.hpp"

#include <exec/async_scope.hpp>
#include <gtest/gtest.h>
#include <stdexec/execution.hpp>

#include <netinet/in.h>
#include <sys/socket.h>

using namespace io::execution;

static int poll_call_count = 0;
static int interruptions = 3;
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  poll_call_count++;
  if (timeout > 0 && poll_call_count <= interruptions)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(8));
    errno = EINTR;
    return -1;
  }

  return 0;
}

class MockPollTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MockPollTest, TestPoll_)
{
  poll_call_count = 0;
  interruptions = 3;
  std::vector<pollfd> list{{.fd = 0, .events = POLLIN, .revents = 0}};
  auto ret = poll_(list, 10);

  EXPECT_LT(poll_call_count, interruptions + 1);
}
