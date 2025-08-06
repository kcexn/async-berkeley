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
 * @file socket.hpp
 * @brief Cross-platform socket abstraction layer with ancillary data support
 *
 * This header provides platform-agnostic socket buffer and message handling
 * for Windows (WinSock2) and POSIX systems. It implements thread-safe
 * ancillary data buffers and message structures that unify the different
 * socket APIs across platforms.
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
 * @brief Cross-platform socket utilities and abstractions
 *
 * Provides unified socket buffer management, message handling, and ancillary
 * data support across Windows and POSIX platforms. The implementation uses
 * CRTP (Curiously Recurring Template Pattern) to provide platform-specific
 * optimizations while maintaining a consistent interface.
 */
namespace iosched::socket {

    /**
     * @namespace iosched::socket::detail
     * @brief Internal implementation details for socket abstractions
     */
    namespace detail {

        /**
         * @class ancillary_buffer_impl
         * @brief Thread-safe ancillary data buffer implementation
         * @tparam Base Platform-specific base class (wsabuf_base or posix_base)
         *
         * Provides a thread-safe container for ancillary data (control messages)
         * sent alongside socket data. Uses CRTP to adapt to platform-specific
         * socket buffer structures while maintaining consistent semantics.
         *
         * Thread Safety: All operations are protected by an internal mutex.
         * Copy/move operations use scoped locking to prevent data races.
         */
        template<typename Base>
        class ancillary_buffer_impl : public Base {
        public:
            /// Type alias for the underlying ancillary data container
            using ancillary_data = std::vector<char>;

            /**
             * @brief Default constructor - creates empty ancillary buffer
             */
            ancillary_buffer_impl() = default;

            /**
             * @brief Copy constructor with thread-safe data transfer
             * @param other The ancillary buffer to copy from
             *
             * Thread-safe copy operation that locks the source buffer
             * during data transfer to prevent corruption.
             */
            ancillary_buffer_impl(const ancillary_buffer_impl& other)
                : ancillary_buffer_impl()
            {
                std::lock_guard lock{other.mtx_};

                // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
                data_ = other.data_;
                update_base();
            }

            /**
             * @brief Move constructor using swap semantics
             * @param other The ancillary buffer to move from
             */
            ancillary_buffer_impl(ancillary_buffer_impl&& other) noexcept
                : ancillary_buffer_impl()
            {
                swap(*this, other);
            }

            /**
             * @brief Construct from existing ancillary data buffer
             * @param buffer The data to initialize the buffer with
             */
            ancillary_buffer_impl(const ancillary_data& buffer)
                : data_{buffer}
            {
                update_base();
            }

            /**
             * @brief Construct from moved ancillary data buffer
             * @param buffer The data to move into the buffer
             */
            ancillary_buffer_impl(ancillary_data&& buffer)
                : data_{std::move(buffer)}
            {
                update_base();
            }

            /**
             * @brief Copy assignment operator using copy-and-swap idiom
             * @param other The ancillary buffer to copy from
             * @return Reference to this object
             */
            auto operator=(const ancillary_buffer_impl& other) -> ancillary_buffer_impl& {
                auto temp = ancillary_buffer_impl{other};
                swap(*this, temp);
                return *this;
            }

            /**
             * @brief Move assignment operator
             * @param other The ancillary buffer to move from
             * @return Reference to this object
             */
            auto operator=(ancillary_buffer_impl&& other) noexcept -> ancillary_buffer_impl& {
                swap(*this, other);
                return *this;
            }

            /**
             * @brief Thread-safe swap operation
             * @param lhs Left-hand side buffer
             * @param rhs Right-hand side buffer
             *
             * Uses scoped locking to safely swap buffer contents between
             * two ancillary_buffer_impl instances without data races.
             */
            friend void swap(ancillary_buffer_impl& lhs, ancillary_buffer_impl& rhs) noexcept {
                std::scoped_lock lock{lhs.mtx_, rhs.mtx_};

                using std::swap;
                swap(lhs.data_, rhs.data_);
                swap(static_cast<Base&>(lhs), static_cast<Base&>(rhs));
            }

            /**
             * @brief Get pointer to the raw ancillary data
             * @return Const pointer to the buffer data
             */
            [[nodiscard]] auto data() const -> const char* {
                return data_.data();
            }

            /**
             * @brief Get the size of the ancillary data buffer
             * @return Size in bytes of the buffer
             */
            [[nodiscard]] auto size() const -> size_t {
                return data_.size();
            }

            /**
             * @brief Default destructor
             */
            ~ancillary_buffer_impl() = default;

        private:
            /// The underlying ancillary data storage
            ancillary_data data_;
            /// Mutex for thread-safe access to the buffer
            mutable std::mutex mtx_;

            /**
             * @brief Update the platform-specific base class with current buffer info
             *
             * Called after buffer modifications to ensure platform-specific
             * structures (like WSABUF on Windows) have current pointers and sizes.
             */
            void update_base();
        };

        /**
         * @class data_buffer_impl
         * @brief Template-based data buffer implementation for cross-platform I/O
         * @tparam Base Platform-specific base class (wsabuf_base or iovec_type)
         *
         * This template class provides a unified interface for data buffers
         * by abstracting over the platform-specific differences between
         * Windows WSABUF and POSIX iovec structures. The template specializations
         * map the generic data() and size() methods to the appropriate platform-specific
         * member variables (buf/len for Windows, iov_base/iov_len for POSIX).
         *
         * Provides a unified interface for data buffers across Windows and POSIX
         * platforms. Uses CRTP to adapt to platform-specific buffer structures
         * while maintaining consistent method names and semantics.
         *
         * The template specializations handle the different member names and
         * types used by WSABUF (Windows) and iovec (POSIX) structures.
         */
        template<typename Base>
        class data_buffer_impl : public Base {
        public:
            /// Base type alias for clarity
            using base_type = Base;

            /**
             * @brief Default constructor
             */
            data_buffer_impl() = default;

            /**
             * @brief Get mutable reference to buffer data pointer
             * @return Reference to the buffer data pointer
             *
             * Platform-specific implementation provided via template specialization.
             */
            auto data() -> auto& {
                return get_data_ptr();
            }

            /**
             * @brief Get mutable reference to buffer length
             * @return Reference to the buffer length
             *
             * Platform-specific implementation provided via template specialization.
             */
            auto size() -> auto& {
                return get_size_ref();
            }

        private:
            /**
             * @brief Get platform-specific data pointer reference
             *
             * Must be specialized for each platform's buffer structure.
             */
            auto get_data_ptr() -> auto&;

            /**
             * @brief Get platform-specific size reference
             *
             * Must be specialized for each platform's buffer structure.
             */
            auto get_size_ref() -> auto&;
        };

    } // namespace detail

    /**
     * @defgroup platform_impl Platform-Specific Implementations
     * @brief Platform-specific socket buffer and address implementations
     *
     * These implementations adapt the generic socket abstractions to work
     * with platform-specific APIs. Windows uses WinSock2 structures while
     * POSIX systems use standard socket structures.
     * @{
     */
    // Platform-specific implementations

#if BOOST_OS_WINDOWS
    /**
     * @brief Windows-specific ancillary buffer base class
     *
     * Inherits from Windows WSABUF structure to provide compatibility
     * with WinSock2 scatter-gather I/O operations.
     */
    struct wsabuf_base : public WSABUF {
        /**
         * @brief Initialize WSABUF with zero length and null pointer
         */
        wsabuf_base() : WSABUF{0, nullptr} {}
    };

    /**
     * @brief Windows-specific base class update implementation
     *
     * Updates the WSABUF fields with current buffer pointer and size.
     * Required for Windows scatter-gather operations.
     */
    template<>
    void detail::ancillary_buffer_impl<wsabuf_base>::update_base() {
        this->len = static_cast<ULONG>(data_.size());
        this->buf = const_cast<char*>(data_.data());
    }

    /// Windows implementation of ancillary buffer
    using ancillary_buffer = detail::ancillary_buffer_impl<wsabuf_base>;
    /// Windows message header implementation (forward declaration)
    using message_header = detail::message_header_impl<wsamsg_base>;
    /// Windows address storage: sockaddr_storage with int length
    using address_type = std::tuple<struct sockaddr_storage, int>;

    /**
     * @brief Windows-specific data buffer specialization for WSABUF
     *
     * Specializes the template to work with Windows WSABUF structure
     * which uses 'buf' for data pointer and 'len' for size.
     */
    template<>
    auto detail::data_buffer_impl<wsabuf_base>::get_data_ptr() -> auto& {
        return this->buf;
    }

    /**
     * @brief Windows-specific size reference specialization for WSABUF
     */
    template<>
    auto detail::data_buffer_impl<wsabuf_base>::get_size_ref() -> auto& {
        return this->len;
    }

    /// Windows implementation of data buffer
    using data_buffer = detail::data_buffer_impl<wsabuf_base>;

#else
    /**
     * @brief POSIX-specific ancillary buffer base class
     *
     * Empty base class for POSIX systems since ancillary data
     * is handled separately from the main data buffer.
     */
    struct posix_base {};

    /**
     * @brief POSIX-specific base class update implementation
     *
     * No-op implementation since POSIX doesn't require updating
     * base class fields for ancillary data.
     */
    template<>
    void detail::ancillary_buffer_impl<posix_base>::update_base() {}

    /// POSIX implementation of ancillary buffer
    using ancillary_buffer = detail::ancillary_buffer_impl<posix_base>;
    /// POSIX address storage: sockaddr_storage with socklen_t length
    using address_type = std::tuple<struct sockaddr_storage, socklen_t>;

    /// Type alias for POSIX iovec structure
    using iovec_type = struct iovec;

    /**
     * @brief POSIX-specific data buffer specialization for iovec
     *
     * Specializes the template to work with POSIX iovec structure
     * which uses 'iov_base' for data pointer and 'iov_len' for size.
     */
    template<>
    auto detail::data_buffer_impl<iovec_type>::get_data_ptr() -> auto& {
        return this->iov_base;
    }

    /**
     * @brief POSIX-specific size reference specialization for iovec
     */
    template<>
    auto detail::data_buffer_impl<iovec_type>::get_size_ref() -> auto& {
        return this->iov_len;
    }

    /// POSIX implementation of data buffer
    using data_buffer = detail::data_buffer_impl<iovec_type>;

#endif
    /// @}

    /**
     * @struct socket_message
     * @brief Unified socket message structure for cross-platform I/O
     *
     * Encapsulates all components needed for advanced socket operations
     * including address information, data buffers, ancillary data,
     * and message flags. Provides a consistent interface across
     * Windows and POSIX platforms.
     */
    struct socket_message {
        /// Socket address information (platform-specific tuple)
        address_type addr;
        /// Main data buffer for send/receive operations
        data_buffer data;
        /// Ancillary (control) data buffer
        ancillary_buffer ancillary;
        /// Message flags for send/receive operations
        std::size_t flags;
    };

} // namespace iosched::socket
#endif
