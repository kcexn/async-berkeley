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
 * @file socket_dialog.hpp
 * @brief Defines the `socket_dialog` struct.
 */
#pragma once
#ifndef IO_SOCKET_DIALOG_HPP
#define IO_SOCKET_DIALOG_HPP
#include "io/detail/concepts.hpp"
#include "io/socket/socket_handle.hpp"

#include <memory>
// Forward declarations
namespace io::execution {
template <Multiplexer Mux> class executor;
} // namespace io::execution

/**
 * @namespace io::socket
 * @brief The `io::socket` namespace provides a cross-platform abstraction for
 * socket-level I/O operations.
 */
namespace io::socket {
/**
 * @brief A dialog that facilitates asynchronous operations on the
 * socket by the executor.
 * @tparam Mux The multiplexer type.
 */
template <Multiplexer Mux> struct socket_dialog {
  /** @brief The executor type for this dialog. */
  using executor_type = ::io::execution::executor<Mux>;
  /** @brief A weak pointer to the executor that owns the socket. */
  std::weak_ptr<executor_type> executor;
  /** @brief A shared pointer to the socket handle. */
  std::shared_ptr<socket_handle> socket;
  /**
   * @brief Checks if the socket_dialog is valid.
   * @returns `true` if the socket_dialog is valid, `false` otherwise.
   */
  [[nodiscard]] explicit operator bool() const noexcept;
};

/**
 * @brief Compares two `socket_dialog` objects.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns A `std::strong_ordering` indicating the result of the comparison.
 * @throws std::invalid_argument if the underlying socket of either side is
 * nullptr.
 */
template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 const socket_dialog<Mux> &rhs) -> std::strong_ordering;

/**
 * @brief Checks for equality between two `socket_dialog` objects.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns `true` if the two objects are equal, `false` otherwise.
 * @throws std::invalid_argument if the underlying socket of either side is
 * nullptr.
 */
template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs,
                const socket_dialog<Mux> &rhs) -> bool;

/**
 * @brief Compares a `socket_dialog` and a `socket_handle`.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns A `std::strong_ordering` indicating the result of the comparison.
 * @throws std::invalid_argument if the underlying socket is nullptr.
 */
template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 const socket_handle &rhs) -> std::strong_ordering;

/**
 * @brief Checks for equality between a `socket_dialog` and a `socket_handle`.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns `true` if the two objects are equal, `false` otherwise.
 * @throws std::invalid_argument if the underlying socket is nullptr.
 */
template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs,
                const socket_handle &rhs) -> bool;

/**
 * @brief Compares a `socket_dialog` and a `native_socket`.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns A `std::strong_ordering` indicating the result of the comparison.
 * @throws std::invalid_argument if the underlying socket is nullptr.
 */
template <Multiplexer Mux>
auto operator<=>(const socket_dialog<Mux> &lhs,
                 native_socket_type rhs) -> std::strong_ordering;

/**
 * @brief Checks for equality between a `socket_dialog` and a `native_socket`.
 * @param lhs The left-hand side of the comparison.
 * @param rhs The right-hand side of the comparison.
 * @returns `true` if the two objects are equal, `false` otherwise.
 * @throws std::invalid_argument if the underlying socket is nullptr.
 */
template <Multiplexer Mux>
auto operator==(const socket_dialog<Mux> &lhs, native_socket_type rhs) -> bool;

} // namespace io::socket

#include "detail/async_operations.hpp" // IWYU pragma: export
#include "impl/socket_dialog_impl.hpp" // IWYU pragma: export

#endif // IO_SOCKET_DIALOG_HPP
