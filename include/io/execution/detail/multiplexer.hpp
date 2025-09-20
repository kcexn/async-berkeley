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
#include "immovable.hpp"
#include "io/detail/concepts.hpp"

#include <variant>
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
  /** @brief The tag type for the multiplexer. */
  using multiplexer_type = Tag;

  /** @brief An intrusive queue of tasks. */
  class intrusive_task_queue {
  public:
    /** @brief An intrusive task that can be executed by the multiplexer. */
    struct task : immovable {
      /** @brief Pointer to next in the intrusive queue. */
      task *next = this;

      /**
       * @brief Tail pointer variant.
       *
       * This variant holds either a tail pointer for an
       * an intrusive queue, or a function pointer for
       * a completion task.
       */
      std::variant<task *, void (*)(task *) noexcept> tail;

      /**
       * @brief Executes the task.
       *
       * A completion function MUST be assigned to
       * the task before execute() is called.
       */
      auto execute() noexcept -> void { std::get<1>(tail)(this); }
    };

    /** @brief Checks if the intrusive task queue is empty. */
    [[nodiscard]] auto is_empty() const noexcept -> bool
    {
      return std::get<0>(head_.tail) == &head_;
    }

    /** @brief Pushes a task onto the back of queue. */
    auto push(task *task) noexcept -> void
    {
      task->next = &head_;
      head_.tail = std::get<0>(head_.tail)->next = task;
    }

    /** @brief Pops a task from the front of the queue. */
    auto pop() noexcept -> task *
    {
      if (std::get<0>(head_.tail) == head_.next)
        head_.tail = &head_;
      return std::exchange(head_.next, head_.next->next);
    }

  private:
    task head_{.next = &head_, .tail = &head_};
  };
};
} // namespace io::execution
#endif // IO_MULTIPLEXER_HPP
