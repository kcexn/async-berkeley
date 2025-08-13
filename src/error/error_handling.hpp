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
 * @file error_handling.hpp
 * @brief Defines macros for creating compile-time error messages.
 *
 * This header provides a set of utility macros to generate formatted error
 * messages that automatically include the file and line number where the
 * error occurs. This is useful for compile-time assertions and debugging.
 */

#pragma once
#ifndef IOSCHED_ERROR_HANDLING_HPP
#define IOSCHED_ERROR_HANDLING_HPP

/**
 * @def IOSCHED_STRINGIFY(x)
 * @brief Converts a macro argument to a string literal.
 * @param x The argument to stringify.
 * @hideinitializer
 */
#define IOSCHED_STRINGIFY(x) #x

/**
 * @def IOSCHED_TOSTRING(x)
 * @brief Helper macro to convert the result of a macro expansion to a string.
 *
 * This is necessary to correctly stringify macros like `__LINE__`, which
 * must be expanded before being converted to a string.
 * @param x The macro to expand and stringify.
 * @hideinitializer
 */
#define IOSCHED_TOSTRING(x) IOSCHED_STRINGIFY(x)

/**
 * @def IOSCHED_ERROR_MESSAGE(msg)
 * @brief Constructs a formatted error message with file and line number.
 *
 * The resulting string literal is in the format `"file:line:message"`.
 *
 * @param msg The custom error message string to append.
 * @return A string literal containing the full error message.
 * @hideinitializer
 *
 * @b Example
 * @code
 * #include "error_handling.hpp"
 * #include <iostream>
 *
 * void some_function() {
 *     std::cerr << IOSCHED_ERROR_MESSAGE("Something went wrong!") << std::endl;
 * }
 * // Possible output:
 * // /path/to/your/file.cpp:123:Something went wrong!
 * @endcode
 */
#define IOSCHED_ERROR_MESSAGE(msg) (__FILE__ ":" IOSCHED_TOSTRING(__LINE__) ":" msg)
#endif // IOSCHED_ERROR_HANDLING_HPP
