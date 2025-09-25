# Copyright 2025 Kevin Exton
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Examples CMakeLists.txt for io library

# Async Ping/Pong Client Example
add_executable(async_ping_pong_client async_ping_pong_client.cpp)
target_include_directories(async_ping_pong_client
    PUBLIC
    "${INCLUDE_DIRS}"
    ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)

# TCP Echo Example
add_executable(tcp_echo tcp_echo.cpp)
target_include_directories(tcp_echo
    PUBLIC
    "${INCLUDE_DIRS}"
    ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)
