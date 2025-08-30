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
 * @file multiplexer.hpp
 * @brief This file provides generic operation state definitions.
 */
#pragma once
#ifndef IO_MULTIPLEXER_HPP
#define IO_MULTIPLEXER_HPP
#include "concepts.hpp"
#include "immovable.hpp"

#include <cassert>
/**
 * @namespace io::execution
 * @brief Provides high-level interfaces for executors and completion triggers.
 */
namespace io::execution {

/**
 * @brief A basic multiplexer that can be used with different event types.
 * @tparam Tag The tag type for the multiplexer.
 */
template <MuxTag Tag> struct basic_multiplexer : public Tag {
  /**
   * @brief A task that can be executed by the multiplexer.
   */
  struct task : public immovable {
    /**
     * @brief A pointer to the completion function.
     */
    void (*complete)(task *) = nullptr;

    /**
     * @brief Executes the task.
     */
    auto execute() -> void {
      assert(complete != nullptr &&
             "execute() must be called with a valid completion handle.");
      complete(this);
    }
  };
};
} // namespace io::execution
#endif // IO_MULTIPLEXER_HPP
