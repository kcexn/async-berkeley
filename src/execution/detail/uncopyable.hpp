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

#pragma once
#ifndef IO_UNCOPYABLE_HPP
#define IO_UNCOPYABLE_HPP

namespace io::execution::detail {

struct uncopyable {
  uncopyable() = default;
  uncopyable(uncopyable &&) = default;
  auto operator=(uncopyable &&) -> uncopyable & = default;
  uncopyable(const uncopyable &) = delete;
  auto operator=(const uncopyable &) -> uncopyable & = delete;
  ~uncopyable() = default;
};

} // namespace io::execution::detail
#endif // IO_UNCOPYABLE_HPP
