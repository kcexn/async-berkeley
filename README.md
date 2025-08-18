#io

[![Build](https://github.com/kcexn/iosched/actions/workflows/build.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/build.yml)
[![Tests](https://github.com/kcexn/iosched/actions/workflows/tests.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

A modern C++20 socket library providing cross-platform socket operations with tag-dispatched customization points, RAII resource management, and thread-safe design patterns.

## Features

- **Tag-Dispatched Operations**: Type-safe, extensible socket operations using the `tag_invoke` customization point pattern
- **Cross-Platform Socket Abstraction**: Unified API for Windows (WinSock2) and POSIX socket operations with platform-specific implementations
- **Thread-Safe Socket Handles**: RAII socket wrappers with atomic storage and mutex-protected operations
- **Move-Only Semantics**: Clear resource ownership preventing double-free errors and resource leaks
- **Comprehensive Socket Operations**: Full set of socket operations (bind, listen, connect, accept, send/recv, etc.) through customization points
- **Exception Safety**: Robust error handling with automatic resource cleanup
- **Modern C++20 Design**: Extensive use of concepts, three-way comparison, and contemporary C++ patterns

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
cmake --preset release     # Optimized build
cmake --preset benchmark   # High-performance build with -O3 -march=native
```

### Detailed Build Instructions

For comprehensive build instructions, dependency installation guides, code coverage setup, and troubleshooting, see [DEVELOPER.md](DEVELOPER.md).

### Documentation

API documentation is available at: [https://kcexn.github.io/iosched/](https://kcexn.github.io/iosched/)

To build documentation locally:
```bash
cmake --preset debug -DIOSCHED_ENABLE_DOCS=ON
cmake --build --preset debug --target docs         # Generate docs
cmake --build --preset debug --target docs-deploy  # Deploy to docs/html/ for GitHub Pages
# View at build/debug/docs/html/index.html or docs/html/index.html
```

## Usage

### Basic Socket Operations

```cpp
#include "io.hpp"
#include "socket/socket_handle.hpp"
#include <netinet/in.h>

// Create a RAII socket handle
io::socket::socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Bind to address using tag-dispatched operation
sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = 0;  // Let system choose port

io::bind(server_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));

// Start listening
io::listen(server_socket, 5);

// Accept incoming connections (low-level API)
sockaddr_in client_addr{};
socklen_t client_len = sizeof(client_addr);
auto client_fd = io::accept(server_socket,
                           reinterpret_cast<sockaddr*>(&client_addr),
                           &client_len);

if (client_fd != -1) {
  // Handle client connection
  io::socket::socket_handle client_socket(client_fd);
  // Socket automatically closed when handle goes out of scope
}

// Alternative: Higher-level API returning socket_handle and socket_address
auto [client_socket, client_address] = io::accept(server_socket);
// Both client_socket and client_address are automatically managed
// Access client address: client_address.data() and *client_address.size()
```

### Client Socket Connection

```cpp
#include "io.hpp"
#include "socket/socket_handle.hpp"
#include <netinet/in.h>

// Create client socket
io::socket::socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Connect to server
sockaddr_in server_addr{};
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
server_addr.sin_port = htons(8080);

int result = io::connect(client_socket,
                        reinterpret_cast<const sockaddr*>(&server_addr),
                        sizeof(server_addr));

if (result == 0) {
  // Connected successfully - send/receive data
  const char *message = "Hello, server!";

  struct iovec iov {};
  iov.iov_base = const_cast<char *>(message);
  iov.iov_len = strlen(message);

  struct msghdr msg {};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  io::sendmsg(client_socket, &msg, 0);
}
```

### Socket Address Management

```cpp
#include "socket/socket_address.hpp"
#include <netinet/in.h>

// Create a socket address wrapper
sockaddr_in native_addr{};
native_addr.sin_family = AF_INET;
native_addr.sin_addr.s_addr = INADDR_ANY;
native_addr.sin_port = htons(8080);

// Wrap in platform-independent socket_address
io::socket::socket_address addr(
    reinterpret_cast<const sockaddr*>(&native_addr),
    sizeof(native_addr)
);

// Use with socket operations
io::socket::socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
io::bind(socket, addr.data(), *addr.size());

// Copy and move semantics work as expected
auto addr_copy = addr;  // Deep copy
auto addr_moved = std::move(addr);  // Move operation
```

### High-Level Socket Operations

The library provides high-level operations that return managed objects for easier resource management:

```cpp
#include "io.hpp"
#include "socket/socket_handle.hpp"
#include "socket/socket_address.hpp"

// Server setup
io::socket::socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Bind using socket_address helper
sockaddr_in bind_addr{};
bind_addr.sin_family = AF_INET;
bind_addr.sin_addr.s_addr = INADDR_ANY;
bind_addr.sin_port = 0;

io::socket::socket_address server_addr(
    reinterpret_cast<const sockaddr*>(&bind_addr), sizeof(bind_addr));
io::bind(server_socket, server_addr);

io::listen(server_socket, 5);

// Accept using high-level API that returns managed objects
try {
    auto [client_socket, client_addr] = io::accept(server_socket);

    // client_socket is a fully managed socket_handle
    // client_addr is a socket_address with the client's address information

    const auto* addr_info = reinterpret_cast<const sockaddr_in*>(client_addr.data());
    std::cout << "Client connected from port: " << ntohs(addr_info->sin_port) << std::endl;

    // Both objects are automatically cleaned up when they go out of scope
} catch (const std::system_error& e) {
    std::cerr << "Accept failed: " << e.what() << std::endl;
}
```

## Architecture

### Core Components

- **`io::socket::socket_handle`**: Thread-safe RAII socket wrapper with atomic storage and mutex-protected operations
- **`io::socket::socket_address`**: Platform-independent socket address abstraction with safe data access via `data()` and `size()` methods
- **Tag-Dispatched Operations**: Extensible socket operations (`io::bind`, `io::connect`, `io::accept`, etc.) with multiple overloads using the `tag_invoke` pattern
- **Cross-Platform Abstraction**: Platform-specific implementations in `src/socket/platforms/` with unified interfaces
- **Error Handling**: Structured exception handling with `std::system_error` for high-level operations and return codes for low-level operations

### Design Principles

- **Tag-Dispatched Customization**: Type-safe, extensible operations using `tag_invoke` pattern for compile-time dispatch
- **Cross-Platform Compatibility**: Unified API with platform-specific implementations using conditional compilation
- **RAII Resource Management**: Automatic socket cleanup with exception-safe constructors and destructors
- **Thread Safety**: Atomic socket storage with mutex protection for modification operations
- **Move-Only Semantics**: Clear resource ownership preventing accidental copies and resource leaks
- **Exception Safety**: Robust error handling with proper cleanup guaranteed in all failure scenarios
- **Modern C++20**: Extensive use of three-way comparison, concepts, and contemporary language features

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
