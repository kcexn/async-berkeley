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
 * @file execution_trigger.hpp
 * @brief Defines the execution_trigger enum for specifying I/O events.
 */
#pragma once
#ifndef IO_EXECUTION_TRIGGER_HPP
#define IO_EXECUTION_TRIGGER_HPP
#include <cstdint>
/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief An enum for specifying I/O events.
 */
enum struct execution_trigger : std::uint8_t {
  /**
   * @brief A read event.
   */
  READ = 1 << 0,
  /**
   * @brief A write event.
   */
  WRITE = 1 << 1
};

} // namespace io::execution
#endif // IO_EXECUTION_TRIGGER_HPP