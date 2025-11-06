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
// NOLINTBEGIN"
#include "io/socket/detail/buffer_iterator.hpp"
#include "io/socket/detail/socket.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <span>
#include <vector>

using namespace io::socket;

class BufferIteratorTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Create test buffers with known data
    data1_ = {1, 2, 3, 4, 5};
    data2_ = {6, 7, 8, 9, 10};
    data3_ = {11, 12, 13};

    // Create native buffers pointing to the data
#if OS_WINDOWS
    buffers_.push_back({static_cast<ULONG>(data1_.size()),
                        reinterpret_cast<CHAR *>(data1_.data())});
    buffers_.push_back({static_cast<ULONG>(data2_.size()),
                        reinterpret_cast<CHAR *>(data2_.data())});
    buffers_.push_back({static_cast<ULONG>(data3_.size()),
                        reinterpret_cast<CHAR *>(data3_.data())});
#else
    buffers_.push_back({static_cast<void *>(data1_.data()), data1_.size()});
    buffers_.push_back({static_cast<void *>(data2_.data()), data2_.size()});
    buffers_.push_back({static_cast<void *>(data3_.data()), data3_.size()});
#endif
  }

  void TearDown() override {}

  std::vector<int> data1_;
  std::vector<int> data2_;
  std::vector<int> data3_;
  std::vector<native_buffer_type> buffers_;
};

// Test iterator concepts
TEST_F(BufferIteratorTest, IteratorConceptRequirements)
{
  using iterator_type =
      buffer_iterator<std::vector<native_buffer_type>::iterator>;

  // Verify it satisfies random_access_iterator concept
  EXPECT_TRUE(std::random_access_iterator<iterator_type>);
  EXPECT_TRUE(std::bidirectional_iterator<iterator_type>);
  EXPECT_TRUE(std::forward_iterator<iterator_type>);
  EXPECT_TRUE(std::input_iterator<iterator_type>);
}

// Test type aliases
TEST_F(BufferIteratorTest, TypeAliases)
{
  using iterator_type =
      buffer_iterator<std::vector<native_buffer_type>::iterator>;

  // Check that type aliases are correctly defined
  EXPECT_TRUE((std::is_same_v<typename iterator_type::value_type,
                              std::span<std::byte>>));
  EXPECT_TRUE((
      std::is_same_v<typename iterator_type::difference_type, std::ptrdiff_t>));
  EXPECT_TRUE((std::is_same_v<typename iterator_type::pointer, void>));
  EXPECT_TRUE((
      std::is_same_v<typename iterator_type::reference, std::span<std::byte>>));
  EXPECT_TRUE((std::is_same_v<typename iterator_type::iterator_concept,
                              std::random_access_iterator_tag>));
}

// Test default construction
TEST_F(BufferIteratorTest, DefaultConstruction)
{
  buffer_iterator<std::vector<native_buffer_type>::iterator> iter;
  // Default constructed iterator exists but shouldn't be dereferenced
  (void)iter; // Suppress unused variable warning
}

// Test construction with underlying iterator
TEST_F(BufferIteratorTest, ConstructionWithIterator)
{
  auto iter = buffer_iterator(buffers_.begin());
  EXPECT_EQ(iter.base(), buffers_.begin());
}

// Test dereference operator
TEST_F(BufferIteratorTest, DereferenceOperator)
{
  auto iter = buffer_iterator(buffers_.begin());

  // Dereference should return a span
  auto span = *iter;
  EXPECT_EQ(span.size(), data1_.size());
  EXPECT_EQ(span.data(), reinterpret_cast<std::byte *>(data1_.data()));

  // Verify span contents
  EXPECT_EQ(std::memcmp(span.data(), data1_.data(), data1_.size()), 0);
}

// Test subscript operator
TEST_F(BufferIteratorTest, SubscriptOperator)
{
  auto iter = buffer_iterator(buffers_.begin());

  // Test subscript access
  auto span0 = iter[0];
  EXPECT_EQ(span0.size(), data1_.size());
  EXPECT_EQ(std::memcmp(span0.data(), data1_.data(), data1_.size()), 0);

  auto span1 = iter[1];
  EXPECT_EQ(span1.size(), data2_.size());
  EXPECT_EQ(std::memcmp(span1.data(), data2_.data(), data2_.size()), 0);

  auto span2 = iter[2];
  EXPECT_EQ(span2.size(), data3_.size());
  EXPECT_EQ(std::memcmp(span2.data(), data3_.data(), data3_.size()), 0);
}

// Test pre-increment
TEST_F(BufferIteratorTest, PreIncrement)
{
  auto iter = buffer_iterator(buffers_.begin());
  auto &result = ++iter;

  // Pre-increment should return reference to itself
  EXPECT_EQ(&result, &iter);
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);

  // Verify new position
  auto span = *iter;
  EXPECT_EQ(std::memcmp(span.data(), data2_.data(), data2_.size()), 0);
}

// Test post-increment
TEST_F(BufferIteratorTest, PostIncrement)
{
  auto iter = buffer_iterator(buffers_.begin());
  auto old = iter++;

  // Post-increment should return copy of old value
  EXPECT_EQ(old.base(), buffers_.begin());
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);

  // Verify old iterator points to first buffer
  auto span_old = *old;
  EXPECT_EQ(std::memcmp(span_old.data(), data1_.data(), span_old.size()), 0);

  // Verify new iterator points to second buffer
  auto span_new = *iter;
  EXPECT_EQ(std::memcmp(span_new.data(), data2_.data(), span_new.size()), 0);
}

// Test pre-decrement
TEST_F(BufferIteratorTest, PreDecrement)
{
  auto iter = buffer_iterator(buffers_.begin() + 2);
  auto &result = --iter;

  // Pre-decrement should return reference to itself
  EXPECT_EQ(&result, &iter);
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);

  // Verify new position
  auto span = *iter;
  EXPECT_EQ(std::memcmp(span.data(), data2_.data(), span.size()), 0);
}

// Test post-decrement
TEST_F(BufferIteratorTest, PostDecrement)
{
  auto iter = buffer_iterator(buffers_.begin() + 2);
  auto old = iter--;

  // Post-decrement should return copy of old value
  EXPECT_EQ(old.base(), buffers_.begin() + 2);
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);

  // Verify old iterator points to third buffer
  auto span_old = *old;
  EXPECT_EQ(std::memcmp(span_old.data(), data3_.data(), span_old.size()), 0);

  // Verify new iterator points to second buffer
  auto span_new = *iter;
  EXPECT_EQ(std::memcmp(span_new.data(), data2_.data(), span_new.size()), 0);
}

// Test addition assignment
TEST_F(BufferIteratorTest, AdditionAssignment)
{
  auto iter = buffer_iterator(buffers_.begin());

  iter += 2;
  EXPECT_EQ(iter.base(), buffers_.begin() + 2);

  auto span = *iter;
  EXPECT_EQ(std::memcmp(span.data(), data3_.data(), span.size()), 0);

  // Test negative offset
  iter += -1;
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);
}

// Test subtraction assignment
TEST_F(BufferIteratorTest, SubtractionAssignment)
{
  auto iter = buffer_iterator(buffers_.begin() + 2);

  iter -= 2;
  EXPECT_EQ(iter.base(), buffers_.begin());

  auto span = *iter;
  EXPECT_EQ(std::memcmp(span.data(), data1_.data(), span.size()), 0);

  // Test negative offset
  iter -= -1;
  EXPECT_EQ(iter.base(), buffers_.begin() + 1);
}

// Test iterator + offset
TEST_F(BufferIteratorTest, AdditionOperator)
{
  auto iter = buffer_iterator(buffers_.begin());

  auto result = iter + 2;
  EXPECT_EQ(result.base(), buffers_.begin() + 2);
  EXPECT_EQ(iter.base(), buffers_.begin()); // Original unchanged

  auto span = *result;
  EXPECT_EQ(std::memcmp(span.data(), data3_.data(), span.size()), 0);
}

// Test offset + iterator (commutative)
TEST_F(BufferIteratorTest, AdditionOperatorReversed)
{
  auto iter = buffer_iterator(buffers_.begin());

  auto result = 2 + iter;
  EXPECT_EQ(result.base(), buffers_.begin() + 2);
  EXPECT_EQ(iter.base(), buffers_.begin()); // Original unchanged

  auto span = *result;
  EXPECT_EQ(std::memcmp(span.data(), data3_.data(), span.size()), 0);
}

// Test iterator - offset
TEST_F(BufferIteratorTest, SubtractionOperator)
{
  auto iter = buffer_iterator(buffers_.begin() + 2);

  auto result = iter - 2;
  EXPECT_EQ(result.base(), buffers_.begin());
  EXPECT_EQ(iter.base(), buffers_.begin() + 2); // Original unchanged

  auto span = *result;
  EXPECT_EQ(std::memcmp(span.data(), data1_.data(), span.size()), 0);
}

// Test iterator difference
TEST_F(BufferIteratorTest, IteratorDifference)
{
  auto iter1 = buffer_iterator(buffers_.begin());
  auto iter2 = buffer_iterator(buffers_.begin() + 2);

  EXPECT_EQ(iter2 - iter1, 2);
  EXPECT_EQ(iter1 - iter2, -2);
  EXPECT_EQ(iter1 - iter1, 0);
}

// Test equality comparison
TEST_F(BufferIteratorTest, EqualityComparison)
{
  auto iter1 = buffer_iterator(buffers_.begin());
  auto iter2 = buffer_iterator(buffers_.begin());
  auto iter3 = buffer_iterator(buffers_.begin() + 1);

  EXPECT_TRUE(iter1 == iter2);
  EXPECT_FALSE(iter1 == iter3);
  EXPECT_TRUE(iter1 != iter3);
  EXPECT_FALSE(iter1 != iter2);
}

// Test three-way comparison
TEST_F(BufferIteratorTest, ThreeWayComparison)
{
  auto iter1 = buffer_iterator(buffers_.begin());
  auto iter2 = buffer_iterator(buffers_.begin() + 1);
  auto iter3 = buffer_iterator(buffers_.begin() + 2);

  EXPECT_TRUE(iter1 < iter2);
  EXPECT_TRUE(iter1 <= iter2);
  EXPECT_TRUE(iter2 > iter1);
  EXPECT_TRUE(iter2 >= iter1);
  EXPECT_TRUE(iter1 <= iter1);
  EXPECT_TRUE(iter1 >= iter1);

  EXPECT_TRUE(iter1 < iter3);
  EXPECT_TRUE(iter3 > iter2);
}

// Test base() method
TEST_F(BufferIteratorTest, BaseMethod)
{
  auto underlying = buffers_.begin();
  auto iter = buffer_iterator(underlying);

  EXPECT_EQ(iter.base(), underlying);

  ++iter;
  EXPECT_EQ(iter.base(), underlying + 1);
}

// Test with standard algorithms
TEST_F(BufferIteratorTest, StandardAlgorithms)
{
  auto begin = buffer_iterator(buffers_.begin());
  auto end = buffer_iterator(buffers_.end());

  // Test std::distance
  EXPECT_EQ(std::distance(begin, end), 3);

  // Test std::advance
  auto iter = begin;
  std::advance(iter, 2);
  EXPECT_EQ(iter.base(), buffers_.begin() + 2);

  // Test std::next
  auto next = std::next(begin, 1);
  auto span = *next;
  EXPECT_EQ(std::memcmp(span.data(), data2_.data(), span.size()), 0);

  // Test std::prev
  auto prev = std::prev(end, 1);
  span = *prev;
  EXPECT_EQ(std::memcmp(span.data(), data3_.data(), span.size()), 0);
}

// Test iteration in range-based for loop
TEST_F(BufferIteratorTest, RangeBasedForLoop)
{
  auto begin = buffer_iterator(buffers_.begin());
  auto end = buffer_iterator(buffers_.end());

  std::size_t count = 0;
  std::vector<std::size_t> sizes;

  for (auto iter = begin; iter != end; ++iter)
  {
    auto span = *iter;
    sizes.push_back(span.size());
    ++count;
  }

  EXPECT_EQ(count, 3);
  EXPECT_EQ(sizes[0], data1_.size());
  EXPECT_EQ(sizes[1], data2_.size());
  EXPECT_EQ(sizes[2], data3_.size());
}

// Test const iterator
TEST_F(BufferIteratorTest, ConstIterator)
{
  using const_iterator_type =
      buffer_iterator<std::vector<native_buffer_type>::const_iterator>;

  const auto &const_buffers = buffers_;
  auto iter = const_iterator_type(const_buffers.begin());

  auto span = *iter;
  EXPECT_EQ(span.size(), data1_.size());
  EXPECT_EQ(std::memcmp(span.data(), data1_.data(), span.size()), 0);
}

// Test empty span handling
TEST_F(BufferIteratorTest, EmptySpan)
{
  std::vector<native_buffer_type> empty_buffers;

#if OS_WINDOWS
  empty_buffers.push_back({0, nullptr});
#else
  empty_buffers.push_back({nullptr, 0});
#endif

  auto iter = buffer_iterator(empty_buffers.begin());
  auto span = *iter;

  EXPECT_EQ(span.size(), 0);
  EXPECT_TRUE(span.empty());
}

// Test copy and move semantics
TEST_F(BufferIteratorTest, CopyAndMove)
{
  auto iter1 = buffer_iterator(buffers_.begin());

  // Copy construction
  auto iter2 = iter1;
  EXPECT_EQ(iter1.base(), iter2.base());

  // Copy assignment
  auto iter3 = buffer_iterator(buffers_.begin() + 1);
  iter3 = iter1;
  EXPECT_EQ(iter1.base(), iter3.base());

  // Move construction
  auto iter4 = std::move(iter2);
  EXPECT_EQ(iter4.base(), iter1.base());

  // Move assignment
  auto iter5 = buffer_iterator(buffers_.begin() + 2);
  iter5 = std::move(iter3);
  EXPECT_EQ(iter5.base(), iter1.base());
}

// Test arithmetic chaining
TEST_F(BufferIteratorTest, ArithmeticChaining)
{
  auto iter = buffer_iterator(buffers_.begin());

  auto result = ((iter + 2) - 1) + 1;
  EXPECT_EQ(result.base(), buffers_.begin() + 2);

  auto span = *result;
  EXPECT_EQ(std::memcmp(span.data(), data3_.data(), span.size()), 0);
}

// Test swap
TEST_F(BufferIteratorTest, Swap)
{
  auto iter1 = buffer_iterator(buffers_.begin());
  auto iter2 = buffer_iterator(buffers_.begin() + 2);

  using std::swap;
  swap(iter1, iter2);

  EXPECT_EQ(iter1.base(), buffers_.begin() + 2);
  EXPECT_EQ(iter2.base(), buffers_.begin());
}
// NOLINTEND
