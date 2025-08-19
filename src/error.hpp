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
 * @file error.hpp
 * @brief This file defines macros for creating compile-time error messages.
 */
#pragma once
#ifndef IO_ERROR_HPP
#define IO_ERROR_HPP

/**
 * @def IO_STRINGIFY(x)
 * @brief Converts a macro argument to a string literal.
 * @param x The argument to stringify.
 */
#define IO_STRINGIFY(x) #x

/**
 * @def IO_TOSTRING(x)
 * @brief Helper macro to convert the result of a macro expansion to a string.
 *
 * This is necessary to correctly stringify macros like `__LINE__`, which must
 * be expanded before being converted to a string.
 * @param x The macro to expand and stringify.
 */
#define IO_TOSTRING(x) IO_STRINGIFY(x)

/**
 * @def IO_ERROR_MESSAGE(msg)
 * @brief Constructs a formatted error message with the file and line number.
 *
 * The resulting string literal is in the format `"file:line: message"`.
 *
 * @param msg The custom error message string to append.
 * @return A string literal containing the full error message.
 *
 * @b Example
 * @code
 * #include "error.hpp"
 * #include <iostream>
 *
 * void some_function() {
 *     std::cerr << IO_ERROR_MESSAGE("Something went wrong!") << std::endl;
 * }
 * // Possible output:
 * // /path/to/your/file.cpp:123: Something went wrong!
 * @endcode
 */
#define IO_ERROR_MESSAGE(msg) (__FILE__ ":" IO_TOSTRING(__LINE__) ": " msg)
#endif // IO_ERROR_HPP
