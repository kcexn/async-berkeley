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
 * @file config.h
 * @brief This file defines various macros that configure AsyncBerkeley.
 */
#pragma once
#ifndef IO_CONFIG_H
#define IO_CONFIG_H

/**
 * @defgroup config Config
 * @brief A collection of configuration macros.
 */

/**
 * @ingroup config
 * @def OS_WINDOWS
 * @brief True if the Windows OS is detected.
 */
#define OS_WINDOWS 0
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||                \
    defined(__TOS_WIN__) || defined(__WINDOWS__)
#undef OS_WINDOWS
#define OS_WINDOWS 1
#endif

/**
 * @ingroup config
 * @def IO_EAGER_ACCEPT
 * @brief True if asynchronous accepts should be eagerly attempted.
 */
#ifndef IO_EAGER_ACCEPT
#define IO_EAGER_ACCEPT 1
#endif

/**
 * @ingroup config
 * @def IO_EAGER_SEND
 * @brief True if asynchronous sends should be eagerly attempted.
 */
#ifndef IO_EAGER_SEND
#define IO_EAGER_SEND 1
#endif

/**
 * @ingroup config
 * @def IO_EAGER_RECV
 * @brief True if asynchronous receives should be eagerly attempted.
 */
#ifndef IO_EAGER_RECV
#define IO_EAGER_RECV 1
#endif

#endif // IO_CONFIG_H
