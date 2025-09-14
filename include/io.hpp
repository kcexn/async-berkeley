/* Copyright 2025 Kevin Exton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is a "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file io.hpp
 * @brief This file contains the main header for the I/O library. It includes
 * all the necessary headers for the I/O operations and defines the main
 * namespace and customization point objects.
 */
#pragma once
#ifndef IO_HPP
#define IO_HPP
#include "io/config.h"                   // IWYU pragma: export
#include "io/detail/customization.hpp"   // IWYU pragma: export
#include "io/execution/multiplexers.hpp" // IWYU pragma: export
#include "io/execution/triggers.hpp"     // IWYU pragma: export
#include "io/socket/socket.hpp"          // IWYU pragma: export
#include "io/socket/socket_address.hpp"  // IWYU pragma: export
#include "io/socket/socket_dialog.hpp"   // IWYU pragma: export
#include "io/socket/socket_handle.hpp"   // IWYU pragma: export
#include "io/socket/socket_message.hpp"  // IWYU pragma: export
#include "io/socket/socket_option.hpp"   // IWYU pragma: export
#endif                                   // IO_HPP
