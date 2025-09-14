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
 * @file macros.h
 * @brief This file defines various macros that are used throughout the I/O
 * library.
 */
#pragma once
#ifndef IO_MACROS_H
#define IO_MACROS_H

/**
 * @defgroup macros Macros
 * @brief A collection of utility macros.
 */

/**
 * @ingroup macros
 * @def IO_STATIC(type)
 * @brief Declares a variable as static in release builds, but not in debug
 * builds.
 *
 * This macro is useful for defining variables that should be static in release
 * builds for optimization purposes, but non-static in debug builds to allow for
 * easier testing and debugging.
 *
 * @param type The type of the variable.
 */
#ifdef NDEBUG
#define IO_STATIC(type) static type
#else
#define IO_STATIC(type) type
#endif

/**
 * @ingroup macros
 * @def OS_WINDOWS
 * @brief True if the Windows OS is detected.
 */
#define OS_WINDOWS 0
#if defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
#undef OS_WINDOWS
#define OS_WINDOWS 1
#endif


#endif // IO_MACROS_H
