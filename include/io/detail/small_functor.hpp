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
 * @file small_functor.hpp
 * @brief This file defines a non-allocating type-erasing functor.
 */
#pragma once
#ifndef IO_SMALL_FUNCTOR_HPP
#define IO_SMALL_FUNCTOR_HPP
#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace io::detail {

/** @brief Declaration of the implementation of `small_functor`. */
template <typename, std::size_t, bool> class small_functor_impl;

/**
 * @brief A type-erased, move-only functor with internal storage.
 *
 * This class can store any callable object that fits within its internal
 * buffer.
 *
 * @note Unlike std::function, this class will NOT allocate if the
 *       callable is larger than its internal storage.
 * @tparam Sig The function signature of the functor. Can be noexcept.
 * @tparam Size The size of the internal buffer in bytes.
 */
template <typename Sig, std::size_t Size> class small_functor;

/**
 * @brief Specialization of `small_functor_impl` for a given function signature.
 * @tparam Ret The return type of the function.
 * @tparam Args The argument types of the function.
 * @tparam Size The size of the internal buffer.
 * @tparam IsNoexcept Whether the function is noexcept.
 */
template <typename Ret, typename... Args, std::size_t Size, bool IsNoexcept>
class small_functor_impl<Ret(Args...), Size, IsNoexcept> {
  /**
   * @brief A vtable for a type-erased callable object.
   *
   * This struct holds function pointers to invoke, destroy, and move the
   * callable object stored in the small_functor.
   */
  struct vtable {
    /** @brief A pointer to a function that can throw exceptions. */
    using invoke_ptr = Ret (*)(void *, Args...);
    /** @brief A pointer to a function that is noexcept. */
    using invoke_ptr_noexcept = Ret (*)(void *, Args...) noexcept;

    /** @brief A pointer to a function that invokes the stored callable. */
    std::conditional_t<IsNoexcept, invoke_ptr_noexcept, invoke_ptr> invoke =
        nullptr;
    /** @brief A pointer to a function that destroys the stored callable. */
    void (*destroy)(void *) noexcept = nullptr;
    /** @brief A pointer to a function that moves the stored callable. */
    void (*move)(void *, void *) noexcept = nullptr;
    /** @brief A pointer to a function that copies the stored callable. */
    void (*copy)(const void *, void *) = nullptr;
  };

  /**
   * @brief A vtable for a given type T.
   * @tparam T The callable type to create the vtable for.
   */
  template <typename T>
  static constexpr vtable vtable_for = {
      .invoke = [](void *ptr, Args &&...args) noexcept(IsNoexcept) -> Ret {
        return (*static_cast<T *>(ptr))(std::forward<Args>(args)...);
      },
      .destroy = [](void *ptr) noexcept { static_cast<T *>(ptr)->~T(); },
      .move =
          [](void *src, void *dst) noexcept {
            new (dst) T(std::move(*static_cast<T *>(src)));
          },
      .copy =
          [] {
            if constexpr (std::is_copy_constructible_v<T>)
            {
              return [](const void *src, void *dst) {
                new (dst) T(*static_cast<const T *>(src));
              };
            }
            else
            {
              return nullptr;
            }
          }()};

public:
  /** @brief Default constructor. Constructs an empty small_functor. */
  small_functor_impl() = default;

  /**
   * @brief Constructs a small_functor from a callable object.
   * @tparam T The type of the callable object.
   * @param callable The callable object to store.
   */
  template <typename T>
    requires(sizeof(T) <= Size) &&
            (IsNoexcept ? std::is_nothrow_invocable_r_v<Ret, T, Args...>
                        : std::is_invocable_r_v<Ret, T, Args...>)
  small_functor_impl(T &&callable) : vptr_(&vtable_for<std::decay_t<T>>)
  {
    new (&storage_) std::decay_t<T>(std::forward<T>(callable));
  }

  /** @brief Copy constructor. */
  small_functor_impl(const small_functor_impl &other)
  {
    if (other.vptr_)
    {
      assert(other.vptr_->copy && "Attempted to copy a non-copyable functor.");
      if (other.vptr_->copy)
      {
        other.vptr_->copy(&other.storage_, &storage_);
        vptr_ = other.vptr_;
      }
    }
  }

  /**
   * @brief Move constructor.
   * @param other The small_functor to move from.
   */
  small_functor_impl(small_functor_impl &&other) noexcept : vptr_(other.vptr_)
  {
    if (vptr_)
      vptr_->move(&other.storage_, &storage_);
    other.vptr_ = nullptr;
  }

  /**
   * @brief Copy assignment operator.
   * @param other The other functor to copy-assign from.
   */
  auto operator=(const small_functor_impl &other) -> small_functor_impl &
  {
    if (this == &other)
      return *this;

    auto tmp = other;
    swap(*this, tmp);
    return *this;
  }

  /**
   * @brief Move assignment operator.
   * @param other The small_functor to move from.
   * @return *this
   */
  auto operator=(small_functor_impl &&other) noexcept -> small_functor_impl &
  {
    swap(*this, other);
    return *this;
  }

  /**
   * @brief Swaps this small_functor with another.
   * @param other The other small_functor to swap with.
   */
  friend void swap(small_functor_impl &lhs, small_functor_impl &rhs) noexcept
  {
    using std::swap;
    if (&lhs == &rhs)
      return;

    alignas(std::max_align_t) std::array<std::byte, Size> temp_storage;

    if (lhs.vptr_)
      lhs.vptr_->move(&lhs.storage_, &temp_storage);

    if (rhs.vptr_)
      rhs.vptr_->move(&rhs.storage_, &lhs.storage_);

    if (lhs.vptr_)
      lhs.vptr_->move(&temp_storage, &rhs.storage_);

    swap(lhs.vptr_, rhs.vptr_);
  }

  /** @brief Destructor. */
  ~small_functor_impl()
  {
    if (vptr_)
      vptr_->destroy(&storage_);
  }

  /**
   * @brief Checks if the small_functor contains a callable object.
   * @return true if the small_functor is not empty, false otherwise.
   */
  explicit operator bool() const noexcept { return vptr_; }

  /**
   * @brief Invokes the stored callable object.
   * @param args The arguments to forward to the callable object.
   * @return The result of the callable object invocation.
   */
  auto operator()(Args &&...args) noexcept(IsNoexcept) -> Ret
  {
    return vptr_->invoke(&storage_, std::forward<Args>(args)...);
  }

private:
  /** @brief The internal storage for the callable object. */
  alignas(std::max_align_t) std::array<std::byte, Size> storage_{};
  /** @brief A pointer to the vtable for the stored callable object. */
  const vtable *vptr_ = nullptr;
};

// Specialization for non-noexcept functions
template <typename Ret, typename... Args, std::size_t Size>
class small_functor<Ret(Args...), Size>
    : public small_functor_impl<Ret(Args...), Size, false> {
  using Base = small_functor_impl<Ret(Args...), Size, false>;

public:
  using Base::Base;
};

// Specialization for noexcept functions
template <typename Ret, typename... Args, std::size_t Size>
class small_functor<Ret(Args...) noexcept, Size>
    : public small_functor_impl<Ret(Args...), Size, true> {
  using Base = small_functor_impl<Ret(Args...), Size, true>;

public:
  using Base::Base;
};

} // namespace io::detail
#endif // IO_SMALL_FUNCTOR_HPP
