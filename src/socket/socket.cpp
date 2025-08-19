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

#include "socket.hpp"

namespace io::socket {

auto tag_invoke([[maybe_unused]] ::io::bind_t tag, const socket_handle &socket,
                const sockaddr_type *addr, socklen_type len) -> int {
  return ::bind(static_cast<native_socket_type>(socket), addr, len);
}

auto tag_invoke([[maybe_unused]] ::io::bind_t tag, const socket_handle &socket,
                const socket_address &addr) -> int {
  return ::bind(static_cast<native_socket_type>(socket), addr.data(),
                *addr.size());
}

auto tag_invoke([[maybe_unused]] ::io::listen_t tag,
                const socket_handle &socket, int backlog) -> int {
  return ::listen(static_cast<native_socket_type>(socket), backlog);
}

auto tag_invoke([[maybe_unused]] ::io::connect_t tag,
                const socket_handle &socket, const sockaddr_type *addr,
                socklen_type len) -> int {
  return ::connect(static_cast<native_socket_type>(socket), addr, len);
}

auto tag_invoke([[maybe_unused]] ::io::connect_t tag,
                const socket_handle &socket,
                const socket_address &addr) -> int {
  return ::connect(static_cast<native_socket_type>(socket), addr.data(),
                   *addr.size());
}

auto tag_invoke([[maybe_unused]] ::io::accept_t tag,
                const socket_handle &socket, sockaddr_type *addr,
                socklen_type *len) -> native_socket_type {
  return ::accept(static_cast<native_socket_type>(socket), addr, len);
}

auto tag_invoke([[maybe_unused]] ::io::accept_t tag,
                const socket_handle &socket, socket_address addr)
    -> std::tuple<socket_handle, socket_address> {
  socket_handle handle{::accept(static_cast<native_socket_type>(socket),
                                addr.data(), addr.size())};
  return std::make_tuple(std::move(handle), addr);
}

auto tag_invoke([[maybe_unused]] ::io::sendmsg_t tag,
                const socket_handle &socket, const socket_message_type *msg,
                int flags) -> std::streamsize {
  return ::io::socket::sendmsg(static_cast<native_socket_type>(socket), msg,
                               flags);
}

auto tag_invoke([[maybe_unused]] ::io::recvmsg_t tag,
                const socket_handle &socket, socket_message_type *msg,
                int flags) -> std::streamsize {
  return ::io::socket::recvmsg(static_cast<native_socket_type>(socket), msg,
                               flags);
}

auto tag_invoke([[maybe_unused]] ::io::getsockopt_t tag,
                const socket_handle &socket, int level, int optname,
                void *optval, socklen_type *optlen) -> int {
  return ::getsockopt(static_cast<native_socket_type>(socket), level, optname,
                      optval, optlen);
}

auto tag_invoke([[maybe_unused]] ::io::setsockopt_t tag,
                const socket_handle &socket, int level, int optname,
                const void *optval, socklen_type optlen) -> int {
  return ::setsockopt(static_cast<native_socket_type>(socket), level, optname,
                      optval, optlen);
}

auto tag_invoke([[maybe_unused]] ::io::getsockname_t tag,
                const socket_handle &socket, sockaddr_type *addr,
                socklen_type *len) -> int {
  return ::getsockname(static_cast<native_socket_type>(socket), addr, len);
}

auto tag_invoke([[maybe_unused]] ::io::getpeername_t tag,
                const socket_handle &socket, sockaddr_type *addr,
                socklen_type *len) -> int {
  return ::getpeername(static_cast<native_socket_type>(socket), addr, len);
}

auto tag_invoke([[maybe_unused]] ::io::shutdown_t tag,
                const socket_handle &socket, int how) -> int {
  return ::shutdown(static_cast<native_socket_type>(socket), how);
}

} // namespace io::socket
