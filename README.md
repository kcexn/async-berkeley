# io

[![Build](https://github.com/kcexn/iosched/actions/workflows/build.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/build.yml)
[![Tests](https://github.com/kcexn/iosched/actions/workflows/tests.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

A high-performance C++20 I/O scheduling library providing cross-platform asynchronous socket operations, event polling, and streaming interfaces with standard library integration.

## Features

- **Cross-Platform Socket Abstraction**: Unified API for Windows (WinSock2) and POSIX socket operations
- **Custom Socket Buffers**: `std::streambuf` implementation with 32KB minimum buffers and dynamic resizing
- **Event Polling**: Template-based polling system using Linux `poll(2)` with O(log n) binary search optimization
- **Stream Interface**: Full C++ `iostream` compatibility for socket operations
- **Thread-Safe Operations**: Mutex-protected ancillary data buffers with RAII management
- **Non-blocking I/O**: Asynchronous socket operations using `MSG_DONTWAIT` flags
- **Memory Efficient**: RAII-based resource management with automatic vector shrinking
- **Policy-Based Design**: Template architecture for extensible polling and trigger systems

## Quick Start

### Prerequisites
- CMake 3.26+
- C++20 compatible compiler
- Boost libraries (header-only Boost.Predef required)

### Build and Test

```bash
# Clone the repository
git clone https://github.com/kcexn/iosched.git
cd iosched

# Quick build with tests
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

# Alternative presets:
# cmake --preset release     # Optimized build
# cmake --preset benchmark   # High-performance build with -O3 -march=native
```

### Detailed Build Instructions

For comprehensive build instructions, dependency installation guides, code coverage setup, and troubleshooting, see [DEVELOPER.md](DEVELOPER.md).

### Documentation

API documentation is available at: [https://kcexn.github.io/iosched/](https://kcexn.github.io/iosched/)

To build documentation locally:
```bash
cmake --preset debug -DIOSCHED_ENABLE_DOCS=ON
cmake --build --preset debug --target docs
# View at build/debug/docs/html/index.html
```

## Usage

### Basic Socket Stream

```cpp
#include "streams.hpp"
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
#include "socket/socket_message.hpp"

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
#include "io.hpp"
#include <chrono>

// Self-contained trigger with embedded poller
iosched::trigger triggers;

// Add socket to polling (binary search insertion for O(log n))
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
triggers.set(sockfd, POLLIN | POLLOUT);

// Poll for events with timeout
auto count = triggers.wait(std::chrono::milliseconds(1000));
if (count > 0) {
  for (const auto &event : triggers.events()) {
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
- **`iosched::socket::socket_handle`**: Thread-safe RAII socket wrapper
- **`iosched::buffers::socket_buffer`**: Socket buffer management with shared ownership

### Design Principles

- **Cross-Platform Compatibility**: Unified API abstracts Windows WinSock2 and POSIX socket differences
- **Policy-Based Templates**: Core classes templated for extensible polling backends (CRTP pattern)
- **Thread Safety**: Mutex-protected operations with scoped locking for concurrent access
- **Non-blocking by Design**: All socket operations use `MSG_DONTWAIT` flags
- **RAII Resource Management**: Automatic cleanup with `std::shared_ptr<socket_buffer>` buffers
- **Standard Library Integration**: Full compatibility with `std::iostream`, algorithms, and containers
- **Memory Optimization**: Automatic vector shrinking when capacity > 8x size and > 256 elements
- **Move-Only Semantics**: Non-copyable socket classes for clear resource ownership

### Dependencies

- **GoogleTest**: Auto-fetched via CMake FetchContent for unit testing
- **Boost.Predef**: Used for cross-platform compiler and OS detection

### Code Quality

The project uses comprehensive static analysis with clang-tidy:

```bash
# Run clang-tidy on the entire codebase
clang-tidy src/**/*.cpp src/**/*.hpp -- -std=c++20 -I src/
```

Configured rules include `bugprone-*`, `cert-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, and `readability-*` checks (excludes `readability-braces-around-statements`, `readability-magic-numbers`, and `readability-implicit-bool-conversion` for flexibility).

## License

This project is licensed under the Apache License, Version 2.0. You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
