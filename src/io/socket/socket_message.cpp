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
 * @file socket_message.cpp
 */
#include "socket_message.hpp"
namespace io::socket {
message_header::operator socket_message_type() {
  // TODO: Windows support.
  return {.msg_name = name.data(),
          .msg_namelen = static_cast<socklen_t>(name.size()),
          .msg_iov = iov.data(),
          .msg_iovlen = iov.size(),
          .msg_control = control.data(),
          .msg_controllen = control.size(),
          .msg_flags = flags};
}

socket_message::operator socket_message_type() {
  Base::operator=({.iov = buffers, .control = control});
  return Base::operator socket_message_type();
}

} // namespace io::socket
