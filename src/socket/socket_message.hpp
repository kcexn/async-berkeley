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
 * @file socket_message.hpp
 * @brief Cross-platform socket message and buffer abstractions.
 *
 * This file defines a set of classes and type aliases for handling socket
 * messages, data buffers, and ancillary data in a platform-independent manner.
 * It uses template metaprogramming (CRTP) and preprocessor checks to abstract
 * away the differences between POSIX (struct iovec, struct msghdr) and
 * Windows (WSABUF, WSAMSG) socket APIs.
 *
 * The primary components are:
 * - `socket_message`: A unified struct for scatter/gather I/O operations.
 * - `ancillary_buffer`: A thread-safe container for control messages.
 * - `data_buffer`: A wrapper for platform-specific data buffer types.
 */
#pragma once
#ifndef IOSCHED_SOCKET
#define IOSCHED_SOCKET
#include <boost/predef.h>

#include <mutex>
#include <tuple>
#include <vector>

#if BOOST_OS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

/**
 * @namespace iosched::socket
 * @brief Provides cross-platform abstractions for socket-level I/O.
 *
 * This namespace contains a set of tools for building and manipulating
 * socket messages and buffers, ensuring that code can be written once and
 * compiled on both Windows and POSIX-compliant systems without modification.
 */
namespace iosched::socket {

/**
 * @namespace iosched::socket::detail
 * @brief Contains internal implementation details for the socket abstractions.
 *
 * The contents of this namespace are not intended for direct use and are
 * subject to change without notice. They provide the underlying mechanics
 * for the public-facing APIs in the `iosched::socket` namespace.
 */
namespace detail {

/**
 * @class ancillary_buffer_impl
 * @brief A thread-safe, platform-aware implementation for ancillary data.
 * @tparam Base The platform-specific base class, which is either
 *              `wsabuf_base` (Windows) or `posix_base` (POSIX).
 *
 * This class manages a buffer for ancillary (control) data used in
 * advanced socket operations. It employs the Curiously Recurring
 * Template Pattern (CRTP) to inherit from a platform-specific base,
 * allowing it to interface directly with native socket APIs.
 *
 * @note Thread safety is ensured by protecting all member data with a
 *       mutex. Copy and swap operations are performed atomically.
 */
template <typename Base> class ancillary_buffer_impl : public Base {
public:
  /// @brief The underlying container for ancillary data bytes.
  using ancillary_data = std::vector<char>;

  /**
   * @brief Constructs an empty ancillary buffer.
   */
  ancillary_buffer_impl() = default;

  /**
   * @brief Thread-safe copy constructor.
   * @param other The object to copy from.
   *
   * A lock is acquired on the other object's mutex during the copy
   * to prevent data races.
   */
  ancillary_buffer_impl(const ancillary_buffer_impl &other)
      : ancillary_buffer_impl() {
    std::lock_guard lock{other.mtx_};

    // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
    data_ = other.data_;
    static_cast<Base &>(*this) = static_cast<const Base &>(other);
  }

  /**
   * @brief Move constructor.
   * @param other The object to move from.
   */
  ancillary_buffer_impl(ancillary_buffer_impl &&other) noexcept
      : ancillary_buffer_impl() {
    swap(*this, other);
  }

  /**
   * @brief Constructs the buffer by copying data from a container.
   * @param buffer The vector of chars to use as the buffer's content.
   */
  ancillary_buffer_impl(const ancillary_data &buffer) : data_{buffer} {
    update_base();
  }

  /**
   * @brief Constructs the buffer by moving data from a container.
   * @param buffer The vector of chars to move into the buffer.
   */
  ancillary_buffer_impl(ancillary_data &&buffer) : data_{std::move(buffer)} {
    update_base();
  }

  /**
   * @brief Copy assignment operator.
   * @param other The object to copy from.
   * @return A reference to this object.
   */
  auto
  operator=(const ancillary_buffer_impl &other) -> ancillary_buffer_impl & {
    auto temp = ancillary_buffer_impl{other};
    swap(*this, temp);
    return *this;
  }

  /**
   * @brief Move assignment operator.
   * @param other The object to move from.
   * @return A reference to this object.
   */
  auto
  operator=(ancillary_buffer_impl &&other) noexcept -> ancillary_buffer_impl & {
    swap(*this, other);
    return *this;
  }

  /**
   * @brief Swaps the contents of two ancillary buffers thread-safely.
   * @param lhs The first buffer.
   * @param rhs The second buffer.
   *
   * This function acquires locks on both buffers to ensure the swap
   * is atomic and avoids deadlocks.
   */
  friend void swap(ancillary_buffer_impl &lhs,
                   ancillary_buffer_impl &rhs) noexcept {
    std::scoped_lock lock{lhs.mtx_, rhs.mtx_};

    using std::swap;
    swap(lhs.data_, rhs.data_);
    swap(static_cast<Base &>(lhs), static_cast<Base &>(rhs));
  }

  /**
   * @brief Gets a pointer to the raw ancillary data.
   * @return A const pointer to the beginning of the data buffer.
   */
  [[nodiscard]] auto data() const -> const char * { return data_.data(); }

  /**
   * @brief Gets the size of the ancillary data buffer.
   * @return The size of the buffer in bytes.
   */
  [[nodiscard]] auto size() const -> size_t { return data_.size(); }

  /**
   * @brief Default destructor.
   */
  ~ancillary_buffer_impl() = default;

private:
  /// @brief The underlying storage for the ancillary data.
  ancillary_data data_;
  /// @brief A mutex to ensure thread-safe access to the buffer.
  mutable std::mutex mtx_;

  /**
   * @brief Updates the platform-specific base class with the current
   *        buffer pointer and size.
   *
   * This function is specialized for each platform. On Windows, it
   * updates the `len` and `buf` members of the `WSABUF` base struct.
   * On POSIX, it is a no-op.
   */
  void update_base();
};

/**
 * @class data_buffer_impl
 * @brief A template-based wrapper for platform-specific data buffers.
 * @tparam Base The native buffer type (`wsabuf_base` or `iovec_type`).
 *
 * This class provides a uniform interface for accessing the data pointer
 * and size of a socket buffer, abstracting away the different field names
 * used by Windows (`buf`, `len`) and POSIX (`iov_base`, `iov_len`).
 *
 * Template specializations of its private methods provide the
 * platform-specific logic.
 */
template <typename Base> class data_buffer_impl : public Base {
public:
  /// @brief A type alias for the base buffer type.
  using base_type = Base;

  /**
   * @brief Default constructor.
   */
  data_buffer_impl() = default;

  /**
   * @brief Gets a mutable reference to the buffer's data pointer.
   * @return A reference to the platform-specific data pointer.
   */
  auto data() -> auto & { return get_data_ptr(); }

  /**
   * @brief Gets a mutable reference to the buffer's length.
   * @return A reference to the platform-specific length field.
   */
  auto size() -> auto & { return get_size_ref(); }

private:
  /**
   * @brief Template-specialized method to get the data pointer.
   * @return A reference to the data pointer member of the base struct.
   */
  auto get_data_ptr() -> auto &;

  /**
   * @brief Template-specialized method to get the size field.
   * @return A reference to the size member of the base struct.
   */
  auto get_size_ref() -> auto &;
};

} // namespace detail

/**
 * @defgroup platform_impl Platform-Specific Implementations
 * @brief Contains platform-specific type definitions and specializations.
 *
 * This section uses preprocessor directives to define types and implement
 * template specializations for Windows and POSIX systems, respectively.
 * @{
 */

#if BOOST_OS_WINDOWS
/**
 * @brief Base struct for ancillary buffers on Windows.
 *
 * Inherits from `WSABUF` to be compatible with the `WSASendMsg` and
 * `WSARecvMsg` functions.
 */
struct wsabuf_base : public WSABUF {
  /**
   * @brief Constructs a zero-initialized `WSABUF`.
   */
  wsabuf_base() : WSABUF{0, nullptr} {}
};

/**
 * @brief Specialization of `update_base` for Windows.
 *
 * Updates the `len` and `buf` members of the `WSABUF` base struct to
 * point to the data stored in the `ancillary_buffer_impl`.
 */
template <> void detail::ancillary_buffer_impl<wsabuf_base>::update_base() {
  this->len = static_cast<ULONG>(data_.size());
  this->buf = const_cast<char *>(data_.data());
}

/// @brief A thread-safe buffer for ancillary data on Windows.
using ancillary_buffer = detail::ancillary_buffer_impl<wsabuf_base>;
/// @brief Platform-specific address type for Windows (address and its length).
using address_type = std::tuple<struct sockaddr_storage, int>;

/**
 * @brief Specialization of `get_data_ptr` for Windows.
 * @return A reference to the `buf` member of the `WSABUF` base struct.
 */
template <>
auto detail::data_buffer_impl<wsabuf_base>::get_data_ptr() -> auto & {
  return this->buf;
}

/**
 * @brief Specialization of `get_size_ref` for Windows.
 * @return A reference to the `len` member of the `WSABUF` base struct.
 */
template <>
auto detail::data_buffer_impl<wsabuf_base>::get_size_ref() -> auto & {
  return this->len;
}

/// @brief A wrapper for a `WSABUF` data buffer on Windows.
using data_buffer = detail::data_buffer_impl<wsabuf_base>;

#else
/**
 * @brief Base struct for ancillary buffers on POSIX systems.
 *
 * This is an empty struct because POSIX systems handle the ancillary
 * data buffer separately from the data buffer (`iovec`).
 */
struct posix_base {};

/**
 * @brief Specialization of `update_base` for POSIX.
 *
 * This is a no-op because the ancillary data buffer is not part of a
 * larger struct that needs updating.
 */
template <> void detail::ancillary_buffer_impl<posix_base>::update_base() {}

/// @brief A thread-safe buffer for ancillary data on POSIX systems.
using ancillary_buffer = detail::ancillary_buffer_impl<posix_base>;
/// @brief Platform-specific address type for POSIX (address and its length).
using address_type = std::tuple<struct sockaddr_storage, socklen_t>;

/// @brief Type alias for the POSIX `iovec` struct.
using iovec_type = struct iovec;

/**
 * @brief Specialization of `get_data_ptr` for POSIX.
 * @return A reference to the `iov_base` member of the `iovec` base struct.
 */
template <>
auto detail::data_buffer_impl<iovec_type>::get_data_ptr() -> auto & {
  return this->iov_base;
}

/**
 * @brief Specialization of `get_size_ref` for POSIX.
 * @return A reference to the `iov_len` member of the `iovec` base struct.
 */
template <>
auto detail::data_buffer_impl<iovec_type>::get_size_ref() -> auto & {
  return this->iov_len;
}

/// @brief A wrapper for an `iovec` data buffer on POSIX systems.
using data_buffer = detail::data_buffer_impl<iovec_type>;

#endif
/** @} */

/**
 * @struct socket_message
 * @brief A unified, cross-platform structure for socket I/O operations.
 *
 * This struct aggregates all the necessary components for scatter/gather
 * I/O (`sendmsg`/`recvmsg` on POSIX, `WSASendMsg`/`WSARecvMsg` on Windows),
 * providing a single, consistent interface for both platforms.
 */
struct socket_message {
  /// @brief The remote address, stored as a platform-specific tuple
  ///        containing a `sockaddr_storage` and its length.
  address_type addr;
  /// @brief The primary data buffer for the I/O operation.
  data_buffer data;
  /// @brief The buffer for ancillary (control) data.
  ancillary_buffer ancillary;
  /// @brief Flags to control message processing (e.g., `MSG_OOB`).
  std::size_t flags;
};

} // namespace iosched::socket
#endif
