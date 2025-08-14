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
#include "socket_dialog.hpp"
#include "../error/error_handling.hpp"

#include <system_error>

namespace iosched::dialog {

auto socket_dialog::bind(const socket_address &address) -> void {
  auto socket_ptr = get();
  if (::bind(*socket_ptr, address.data(), *address.size()))
    throw std::system_error(errno, std::generic_category(),
                            IOSCHED_ERROR_MESSAGE("Failed to bind socket."));
}

} // namespace iosched::dialog
