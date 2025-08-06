# iosched

[![Build](https://github.com/kcexn/iosched/actions/workflows/build.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/build.yml)
[![Tests](https://github.com/kcexn/iosched/actions/workflows/tests.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/tests.yml)
[![codecov](https://codecov.io/gh/kcexn/iosched/branch/main/graph/badge.svg)](https://codecov.io/gh/kcexn/iosched)

A high-performance C++17 I/O scheduling library providing cross-platform asynchronous socket operations, event polling, and streaming interfaces with standard library integration.

## Features

- **Cross-Platform Socket Abstraction**: Unified API for Windows (WinSock2) and POSIX socket operations
- **Custom Socket Buffers**: `std::streambuf` implementation with 32KB minimum buffers and dynamic resizing
- **Event Polling**: Template-based polling system using Linux `poll(2)` with O(log n) binary search optimization
- **Stream Interface**: Full C++ `iostream` compatibility for socket operations
- **Thread-Safe Operations**: Mutex-protected ancillary data buffers with RAII management
- **Non-blocking I/O**: Asynchronous socket operations using `MSG_DONTWAIT` flags
- **Memory Efficient**: RAII-based resource management with automatic vector shrinking
- **Policy-Based Design**: Template architecture for extensible polling and trigger systems

## Building

### Requirements

- CMake 3.26+
- C++17 compatible compiler
- Boost libraries (system-wide installation required)
- Linux/Windows (cross-platform socket support)
- Ninja build system (recommended) or Unix Makefiles

### Installing Dependencies

#### Boost Installation

The project requires Boost libraries to be installed system-wide. Here are installation instructions for common platforms:

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libboost-all-dev
# Or minimal installation:
sudo apt-get install libboost-dev
```

**Red Hat/CentOS/Fedora:**
```bash
# Fedora
sudo dnf install boost-devel

# CentOS/RHEL
sudo yum install boost-devel
```

**macOS (using Homebrew):**
```bash
brew install boost
```

**Windows (using vcpkg):**
```cmd
vcpkg install boost:x64-windows
# Then set CMAKE_TOOLCHAIN_FILE to vcpkg.cmake when configuring
```

**Windows (using Conan):**
```bash
pip install conan
conan install boost/1.82.0@ --build=missing
```

**Building Boost from source (any platform):**
```bash
# Download Boost 1.82+ from https://www.boost.org/
wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz
tar -xzf boost_1_82_0.tar.gz
cd boost_1_82_0
./bootstrap
./b2 --prefix=/usr/local install
```

#### Additional Tools (for development)

**Code Coverage (optional):**
```bash
pip install gcovr
```

**Static Analysis (optional):**
```bash
# Ubuntu/Debian
sudo apt-get install clang-tidy

# macOS
brew install llvm
```

### Quick Start (CMake Presets - Recommended)

```bash
# Configure debug build with tests enabled
cmake --preset debug

# Build the project
cmake --build --preset debug

# Run tests
ctest --preset debug

# Generate code coverage reports (HTML)
cmake --build --preset debug --target coverage
# View coverage report: build/debug/coverage/index.html

# For high-performance benchmarking
cmake --preset benchmark
cmake --build --preset benchmark
```

### Using Unix Makefiles (Alternative)

If you prefer Unix Makefiles over Ninja, create a `CMakeUserPresets.json` file in the project root:

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "debug-make",
            "displayName": "Debug (Unix Makefiles)",
            "description": "Debug build with tests enabled using Unix Makefiles.",
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build/debug-make",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "IOSCHED_ENABLE_TESTS": "ON"
            }
        },
        {
            "name": "release-make",
            "displayName": "Release (Unix Makefiles)",
            "description": "Optimized release build using Unix Makefiles.",
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build/release-make",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug-make",
            "description": "debug build with tests using make",
            "displayName": "Debug (Unix Makefiles)",
            "configurePreset": "debug-make"
        },
        {
            "name": "release-make",
            "description": "optimized release build using make",
            "displayName": "Release (Unix Makefiles)",
            "configurePreset": "release-make"
        }
    ],
    "testPresets": [
        {
            "name": "debug-make",
            "description": "tests using make build",
            "displayName": "Debug (Unix Makefiles)",
            "configurePreset": "debug-make"
        }
    ]
}
```

Then use the new presets:

```bash
# Configure and build with Unix Makefiles
cmake --preset debug-make
cmake --build --preset debug-make

# Run tests
ctest --preset debug-make
```

### Manual Build

```bash
# Basic build
mkdir build && cd build
cmake ..
cmake --build .

# With tests enabled
cmake .. -DIOSCHED_ENABLE_TESTS=ON
cmake --build .

# If Boost is not found, specify the path manually:
cmake .. -DBOOST_ROOT=/path/to/boost -DBoost_NO_SYSTEM_PATHS=TRUE
```

### Troubleshooting

**Boost not found errors:**
```bash
# If CMake cannot find Boost, try specifying the installation path:
cmake .. -DBOOST_ROOT=/usr/local -DBoost_NO_SYSTEM_PATHS=TRUE

# For custom Boost installations:
cmake .. -DBOOST_INCLUDEDIR=/path/to/boost/include -DBOOST_LIBRARYDIR=/path/to/boost/lib

# On Windows with vcpkg:
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Minimum Boost version issues:**
The project only requires Boost.Predef (header-only), so any recent Boost version (1.70+) should work.

### Running Tests

```bash
# Using presets
ctest --preset debug

# Manual approach
cd build
ctest
# Or run specific test:
./tests/socket_test
```

## Usage

### Basic Socket Stream

```cpp
#include "src/streams.hpp"
#include <iostream>

// Create a TCP socket stream
iosched::streams::sockstream sock(AF_INET, SOCK_STREAM, 0);

// Connect to server
struct sockaddr_in addr = {};
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

auto buffer = sock.connectto((struct sockaddr*)&addr, sizeof(addr));
if (buffer && !sock.err()) {
    sock << "Hello, server!" << std::endl;
    
    std::string response;
    std::getline(sock, response);
    std::cout << "Received: " << response << std::endl;
}
```

### Cross-Platform Socket Messages

```cpp
#include "src/socket.hpp"

// Create a unified socket message for cross-platform I/O
iosched::socket::socket_message msg;

// Set up main data buffer (works on both Windows and POSIX)
std::vector<char> data(1024);
msg.data.data() = data.data();
msg.data.size() = data.size();

// Add ancillary data (thread-safe)
std::vector<char> ancillary_data(64);
msg.ancillary = iosched::socket::ancillary_buffer(ancillary_data);

// The message structure adapts automatically to platform:
// - Windows: Uses WSABUF structures with WinSock2
// - POSIX: Uses iovec structures with standard sockets
```

### Event Polling

```cpp
#include "src/io.hpp"
#include <chrono>

// Self-contained trigger with embedded poller
iosched::trigger triggers;

// Add socket to polling (binary search insertion for O(log n))
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
triggers.set(sockfd, POLLIN | POLLOUT);

// Poll for events with timeout
auto count = triggers.wait(std::chrono::milliseconds(1000));
if (count > 0) {
    for (const auto& event : triggers.events()) {
        if (event.revents & POLLIN) {
            // Handle readable socket
        }
        if (event.revents & POLLOUT) {
            // Handle writable socket  
        }
    }
}

// Clear specific events or remove socket entirely
triggers.clear(sockfd, POLLOUT);  // Remove write interest
triggers.clear(sockfd);           // Remove socket completely
```

## Architecture

### Core Components

- **`iosched::buffers::sockbuf`**: Custom `std::streambuf` with dynamic 32KB+ buffers, handles TCP/UDP
- **`iosched::basic_poller<T>`**: Template-based event polling using Linux `poll(2)` 
- **`iosched::basic_trigger<T>`**: Event trigger management with O(log n) binary search on sorted handle lists
- **`iosched::streams::sockstream`**: Full `std::iostream` wrapper with stream operators
- **`iosched::socket::socket_message`**: Cross-platform socket message abstraction with unified API
- **`iosched::socket::ancillary_buffer`**: Thread-safe ancillary data buffers for control messages
- **`iosched::socket::data_buffer`**: Platform-agnostic data buffer (WSABUF/iovec abstraction)

### Design Principles

- **Cross-Platform Compatibility**: Unified API abstracts Windows WinSock2 and POSIX socket differences
- **Policy-Based Templates**: Core classes templated for extensible polling backends (CRTP pattern)
- **Thread Safety**: Mutex-protected operations with scoped locking for concurrent access
- **Non-blocking by Design**: All socket operations use `MSG_DONTWAIT` flags
- **RAII Resource Management**: Automatic cleanup with `std::shared_ptr<socket_message>` buffers
- **Standard Library Integration**: Full compatibility with `std::iostream`, algorithms, and containers
- **Memory Optimization**: Automatic vector shrinking when capacity > 8x size and > 256 elements
- **Move-Only Semantics**: Non-copyable socket classes for clear resource ownership

### Dependencies

- **GoogleTest**: Auto-fetched via CMake FetchContent for unit testing
- **NVIDIA stdexec**: Included via CPM.cmake for future execution model support
- **Boost.Predef**: Used for cross-platform compiler and OS detection

### Code Quality

The project uses comprehensive static analysis with clang-tidy:

```bash
# Run clang-tidy on the entire codebase
clang-tidy src/**/*.cpp src/**/*.hpp -- -std=c++17 -I src/
```

Configured rules include `bugprone-*`, `cert-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, and `readability-*` checks.

## License

This project is licensed under the Apache License, Version 2.0. You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
