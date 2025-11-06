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

/**
 * @file buffer_iterator.hpp
 * @brief Declares a proxy iterator for socket message buffers.
 */
#pragma once
#ifndef IO_BUFFER_ITERATOR_HPP
#define IO_BUFFER_ITERATOR_HPP
#include <iterator>
#include <span>
namespace io::socket {
/**
 * @brief A proxy random access iterator that wraps native_buffer_type iterators
 * and dereferences to std::span<std::byte>.
 *
 * @tparam Iterator An iterator type that points to native_buffer_type elements
 * (e.g., std::vector<native_buffer_type>::iterator).
 *
 * @details
 * This class implements a proxy iterator pattern that provides an abstraction
 * over platform-specific native buffer types (iovec on POSIX, WSABUF on
 * Windows). Instead of directly exposing the underlying native_buffer_type
 * reference when dereferenced, this iterator returns a std::span<std::byte>,
 * providing a uniform, platform-independent interface for accessing buffer
 * data.
 *
 * ## Proxy Iterator Pattern
 * A proxy iterator is one where dereferencing returns a temporary value (proxy
 * object) rather than a reference to an actual stored element. This is
 * necessary because:
 * - The underlying native_buffer_type has platform-specific structure
 * - We want to present a consistent std::span interface across platforms
 * - The span is constructed on-the-fly from the native buffer's pointer and
 * size
 *
 * ## Native Buffer Type Mapping
 * The iterator converts platform-specific buffer structures to std::span:
 * - **POSIX (iovec)**: Extracts iov_base (pointer) and iov_len (size)
 * - **Windows (WSABUF)**: Extracts buf (pointer) and len (size)
 *
 * ## Iterator Requirements
 * This class satisfies the C++20 std::random_access_iterator concept, providing
 * all required operations including random access, arithmetic, and comparisons.
 *
 * ## Usage Example
 * @code
 * std::vector<native_buffer_type> buffers = {...};
 * buffer_iterator iter(buffers.begin());
 *
 * // Dereference returns std::span<std::byte>
 * std::span<std::byte> data = *iter;
 *
 * // Random access via subscript
 * std::span<std::byte> third = iter[2];
 *
 * // Iterator arithmetic
 * auto next = iter + 1;
 * auto distance = next - iter;
 * @endcode
 *
 * @note The pointer type alias is void because this is a proxy iterator - there
 * is no persistent address for the temporary span objects returned on
 * dereference.
 */
template <typename Iterator> class buffer_iterator {
public:
  /** @brief Identifies this as a random access iterator for C++20 iterator
   * concepts. */
  using iterator_concept = std::random_access_iterator_tag;
  /** @brief Iterator category for legacy iterator traits compatibility. */
  using iterator_category = std::random_access_iterator_tag;
  /** @brief The type of value obtained when dereferencing the iterator. */
  using value_type = std::span<std::byte>;
  /** @brief Difference type. */
  using difference_type = std::ptrdiff_t;
  /** @brief Pointer type for iterator operations. */
  using pointer = void;
  /** @brief The reference type returned when dereferencing the iterator. */
  using reference = value_type;

  /** @brief Default constructor creates an uninitialized iterator. */
  constexpr buffer_iterator() noexcept = default;

  /**
   * @brief Constructs a buffer_iterator wrapping an underlying
   * native_buffer_type iterator.
   * @param iter An iterator to native_buffer_type elements (e.g., from a
   * std::vector<native_buffer_type>).
   * @details The iterator is stored internally and will be used to access the
   * underlying native buffer structures when dereferencing operations occur.
   */
  constexpr explicit buffer_iterator(Iterator iter) noexcept;

  /**
   * @brief Dereferences the iterator to obtain a span view of the buffer data.
   * @return std::span<std::byte> A non-owning view of the buffer's memory
   * region.
   * @details Converts the platform-specific native_buffer_type pointed to by
   * the underlying iterator into a std::span<std::byte>. On POSIX systems,
   * extracts iov_base and iov_len from iovec. On Windows, extracts buf and len
   * from WSABUF.
   * @note This returns a temporary span object (proxy) rather than a reference.
   */
  [[nodiscard]] constexpr auto operator*() const noexcept -> reference;

  /**
   * @brief Accesses the element at a given offset from the current iterator
   * position.
   * @param n The offset from the current position (can be negative).
   * @return std::span<std::byte> A span view of the buffer at position current
   * + n.
   * @details Equivalent to *(iter + n). Provides random access to buffer
   * elements without modifying the iterator itself.
   */
  [[nodiscard]] constexpr auto
  operator[](difference_type n) const noexcept -> reference;

  /**
   * @brief Pre-increment operator advances the iterator to the next buffer.
   * @return buffer_iterator& Reference to this iterator after incrementing.
   * @details Advances the underlying iterator by one position and returns a
   * reference to the modified iterator.
   */
  constexpr auto operator++() noexcept -> buffer_iterator &;

  /**
   * @brief Post-increment operator advances the iterator to the next buffer.
   * @param int Dummy parameter to distinguish from pre-increment.
   * @return buffer_iterator A copy of the iterator before incrementing.
   * @details Creates a copy of the current iterator, advances this iterator by
   * one position, and returns the copy.
   */
  constexpr auto operator++(int) noexcept -> buffer_iterator;

  /**
   * @brief Pre-decrement operator moves the iterator to the previous buffer.
   * @return buffer_iterator& Reference to this iterator after decrementing.
   * @details Moves the underlying iterator back by one position and returns a
   * reference to the modified iterator.
   */
  constexpr auto operator--() noexcept -> buffer_iterator &;

  /**
   * @brief Post-decrement operator moves the iterator to the previous buffer.
   * @param int Dummy parameter to distinguish from pre-decrement.
   * @return buffer_iterator A copy of the iterator before decrementing.
   * @details Creates a copy of the current iterator, moves this iterator back
   * by one position, and returns the copy.
   */
  constexpr auto operator--(int) noexcept -> buffer_iterator;

  /**
   * @brief Advances the iterator by a specified offset.
   * @param n The number of positions to advance (can be negative to move
   * backward).
   * @return buffer_iterator& Reference to this iterator after advancing.
   * @details Modifies the iterator in-place by advancing the underlying
   * iterator by n positions.
   */
  constexpr auto operator+=(difference_type n) noexcept -> buffer_iterator &;

  /**
   * @brief Moves the iterator backward by a specified offset.
   * @param n The number of positions to move backward (can be negative to
   * advance).
   * @return buffer_iterator& Reference to this iterator after moving.
   * @details Modifies the iterator in-place by moving the underlying iterator
   * backward by n positions.
   */
  constexpr auto operator-=(difference_type n) noexcept -> buffer_iterator &;

  /**
   * @brief Creates a new iterator advanced by a specified offset.
   * @param iter The iterator to advance.
   * @param n The number of positions to advance (can be negative).
   * @return buffer_iterator A new iterator at position iter + n.
   * @details Creates a copy of iter, advances it by n positions, and returns
   * the result. Does not modify the original iterator. Equivalent to (iter +=
   * n).
   */
  constexpr friend auto operator+(buffer_iterator iter,
                                  difference_type n) noexcept -> buffer_iterator
  {
    return iter += n;
  }

  /**
   * @brief Creates a new iterator advanced by a specified offset (reversed
   * operands).
   * @param n The number of positions to advance (can be negative).
   * @param iter The iterator to advance.
   * @return buffer_iterator A new iterator at position n + iter.
   * @details Provides commutative addition: n + iter is equivalent to iter + n.
   * Allows natural arithmetic expressions like (5 + begin()).
   */
  constexpr friend auto
  operator+(difference_type n, buffer_iterator iter) noexcept -> buffer_iterator
  {
    return iter + n;
  }

  /**
   * @brief Creates a new iterator moved backward by a specified offset.
   * @param iter The iterator to move backward.
   * @param n The number of positions to move backward (can be negative to
   * advance).
   * @return buffer_iterator A new iterator at position iter - n.
   * @details Creates a copy of iter, moves it backward by n positions, and
   * returns the result. Does not modify the original iterator. Equivalent to
   * (iter -= n).
   */
  constexpr friend auto operator-(buffer_iterator iter,
                                  difference_type n) noexcept -> buffer_iterator
  {
    return iter -= n;
  }

  /**
   * @brief Computes the distance between two iterators.
   * @param lhs The iterator to subtract from.
   * @param rhs The iterator to subtract.
   * @return difference_type The number of positions between lhs and rhs.
   * @details Calculates how many positions lhs is ahead of rhs. Returns a
   * positive value if lhs > rhs, negative if lhs < rhs, and zero if lhs == rhs.
   * @note Behavior is undefined if the iterators don't refer to the same
   * container.
   */
  constexpr friend auto
  operator-(const buffer_iterator &lhs,
            const buffer_iterator &rhs) noexcept -> difference_type
  {
    return lhs.it_ - rhs.it_;
  }

  /**
   * @brief Tests whether two iterators are equal.
   * @param lhs The left-hand side iterator.
   * @param rhs The right-hand side iterator.
   * @return bool True if the iterators point to the same position, false
   * otherwise.
   * @details Two buffer_iterators are equal if their underlying iterators are
   * equal. This comparison is synthesized by C++20 to also provide operator!=.
   */
  constexpr friend auto operator==(const buffer_iterator &lhs,
                                   const buffer_iterator &rhs) noexcept -> bool
  {
    return lhs.it_ == rhs.it_;
  }

  /**
   * @brief Performs three-way comparison between two iterators.
   * @param lhs The left-hand side iterator.
   * @param rhs The right-hand side iterator.
   * @return auto A comparison category indicating the relative ordering
   * (typically std::strong_ordering).
   * @details Compares the underlying iterators using operator<=>. Returns:
   * - less than zero if lhs < rhs
   * - equal to zero if lhs == rhs
   * - greater than zero if lhs > rhs
   * C++20 synthesizes operator<, operator<=, operator>, and operator>= from
   * this.
   */
  constexpr friend auto operator<=>(const buffer_iterator &lhs,
                                    const buffer_iterator &rhs) noexcept
  {
    return lhs.it_ <=> rhs.it_;
  }

  /**
   * @brief Provides access to the underlying wrapped iterator.
   * @return Iterator A copy of the underlying iterator to native_buffer_type
   * elements.
   * @details Useful for interoperating with code that needs direct access to
   * the underlying iterator, or for debugging purposes. The returned iterator
   * points to the native_buffer_type structure currently referenced by this
   * buffer_iterator.
   */
  [[nodiscard]] constexpr auto base() const noexcept -> Iterator;

private:
  /** @brief The underlying iterator to native_buffer_type elements. */
  Iterator it_;
};

} // namespace io::socket

#include "impl/buffer_iterator_impl.hpp" // IWYU pragma: export

#endif // IO_BUFFER_ITERATOR_HPP
