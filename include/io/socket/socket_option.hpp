/**
 * @file socket_option.hpp
 * @brief Defines a generic socket option wrapper.
 * @author Kevin Exton
 * @copyright Copyright 2025 Kevin Exton
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
#pragma once
#ifndef IO_SOCKET_OPTION_HPP
#define IO_SOCKET_OPTION_HPP
#include <cassert>
#include <cstring>
#include <span>
namespace io::socket {

/**
 * @class socket_option
 * @brief A generic wrapper for socket options.
 * @tparam T The type of the socket option value.
 */
template <typename T> class socket_option {
public:
  /**
   * @brief The type of the socket option value.
   */
  using value_type = std::decay_t<T>;
  /**
   * @brief The size type for the socket option.
   */
  using size_type = std::size_t;

  /**
   * @brief Constructs a socket_option with a given size.
   * @tparam Size The size of the option.
   * @param size The size of the option in bytes.
   */
  template <size_type Size = sizeof(value_type)>
    requires(Size <= sizeof(value_type))
  constexpr socket_option(size_type size = Size) noexcept : size_{size}
  {}

  /**
   * @brief Default copy constructor.
   */
  socket_option(const socket_option &) = default;
  /**
   * @brief Default move constructor.
   */
  socket_option(socket_option &&) = default;

  /**
   * @brief Constructs a socket_option from a value.
   * @param val The value of the socket option.
   */
  socket_option(const value_type &val) noexcept : size_{sizeof(val)}
  {
    std::memcpy(storage_.data(), &val, size_);
  }

  /**
   * @brief Constructs a socket_option from a span of bytes.
   * @tparam Size The size of the span.
   * @param option The span of bytes representing the option.
   */
  template <size_type Size>
    requires(Size <= sizeof(value_type) || Size == std::dynamic_extent)
  socket_option(std::span<const std::byte, Size> option) noexcept
      : size_{option.size()}
  {
    assert(option.size() <= sizeof(value_type) &&
           "option.size() must be <= sizeof(value_type)");
    assert(option.size() > 0 && option.data() != nullptr &&
           "If option.size() > 0 then option.data() must not be NULL.");
    std::memcpy(storage_.data(), option.data(), size_);
  }

  /**
   * @brief Constructs a socket_option from a span of bytes.
   * @tparam Size The size of the span.
   * @param option The span of bytes representing the option.
   */
  template <size_type Size>
    requires(Size <= sizeof(value_type) || Size == std::dynamic_extent)
  socket_option(std::span<std::byte, Size> option) noexcept
      : socket_option(std::span<const std::byte, Size>(option))
  {}

  /**
   * @brief Default copy assignment operator.
   * @return A reference to this object.
   */
  auto operator=(const socket_option &) -> socket_option & = default;

  /**
   * @brief Default move assignment operator.
   * @return A reference to this object.
   */
  auto operator=(socket_option &&) -> socket_option & = default;

  /**
   * @brief Default destructor.
   */
  ~socket_option() = default;

  /**
   * @brief Dereferences the socket option to its value.
   * @return A reference to the socket option's value.
   */
  [[nodiscard]] constexpr auto operator*() noexcept -> value_type &
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<value_type *>(storage_.data());
  }

  /**
   * @brief Accesses the socket option's value.
   * @return A pointer to the socket option's value.
   */
  [[nodiscard]] constexpr auto operator->() noexcept -> value_type *
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<value_type *>(storage_.data());
  }

  // GCOVR_EXCL_START
  /**
   * @brief Gets an iterator to the beginning of the option's byte
   * representation.
   * @return A pointer to the beginning of the byte storage.
   */
  [[nodiscard]] constexpr auto begin() noexcept -> std::byte *
  {
    return storage_.begin();
  }

  /**
   * @brief Gets a const iterator to the beginning of the option's byte
   * representation.
   * @return A const pointer to the beginning of the byte storage.
   */
  [[nodiscard]] constexpr auto begin() const noexcept -> const std::byte *
  {
    return storage_.cbegin();
  }

  /**
   * @brief Gets an iterator to the end of the option's byte representation.
   * @return A pointer to the end of the byte storage.
   */
  [[nodiscard]] constexpr auto end() noexcept -> std::byte *
  {
    return storage_.begin() + size_;
  }

  /**
   * @brief Gets a const iterator to the end of the option's byte
   * representation.
   * @return A const pointer to the end of the byte storage.
   */
  [[nodiscard]] constexpr auto end() const noexcept -> const std::byte *
  {
    return storage_.cbegin() + size_;
  }
  // GCOVR_EXCL_STOP

  /**
   * @brief Compares this socket_option with another for ordering.
   * @param other The other socket_option to compare with.
   * @return The result of the lexicographical comparison, first by size, then
   * by the raw byte data.
   */
  auto operator<=>(const socket_option &other) const noexcept -> bool
  {
    return (size_ == other.size_)
               ? std::memcmp(storage_.data(), other.storage_.data(), size_)
               : size_ <=> other.size_;
  }

  /**
   * @brief Compares two socket_option objects for equality.
   * @param other The other socket_option to compare against.
   * @return True if the options are equal, false otherwise.
   */
  auto operator==(const socket_option &other) const noexcept -> bool
  {
    return size_ == other.size_ &&
           std::memcmp(storage_.data(), other.storage_.data(), size_) == 0;
  }

private:
  /**
   * @brief The raw byte storage for the option value.
   */
  std::array<std::byte, sizeof(value_type)> storage_{};
  /**
   * @brief The actual size of the option value in bytes.
   */
  size_type size_{sizeof(value_type)};
};

} // namespace io::socket
#endif // IO_SOCKET_OPTION_HPP
