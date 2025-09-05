# iosched

[![Build](https://github.com/kcexn/iosched/actions/workflows/build.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/build.yml)
[![Tests](https://github.com/kcexn/iosched/actions/workflows/tests.yml/badge.svg)](https://github.com/kcexn/iosched/actions/workflows/tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/iosched/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

A modern C++20 I/O scheduling library providing cross-platform asynchronous socket operations, event polling, streaming interfaces, and execution framework with sender/receiver patterns. Built with advanced C++ patterns including tag-dispatched customization points, RAII resource management, persistent socket error tracking, and asynchronous execution with poll multiplexing.

## Features

- **Tag-Dispatched Operations**: Type-safe, extensible socket operations using the `tag_invoke` customization point pattern
- **Cross-Platform Socket Abstraction**: Unified API for Windows (WinSock2) and POSIX socket operations with platform-specific implementations
- **Thread-Safe Socket Handles**: RAII socket wrappers with atomic storage, mutex-protected operations, and persistent error tracking across asynchronous boundaries
- **Advanced Message Handling**: Thread-safe `socket_message` class for scatter-gather I/O and ancillary data with `push()` and `emplace()` methods for efficient data appending
- **Asynchronous Execution Framework**: Sender/receiver pattern implementation with poll multiplexer for scalable I/O operations
- **Event Polling and Triggers**: High-performance I/O event handling with shared_ptr-based lifetime management and execution triggers
- **Move-Only Semantics**: Clear resource ownership preventing double-free errors and resource leaks
- **Comprehensive Socket Operations**: Full set of socket operations (bind, listen, connect, accept, sendmsg, recvmsg, etc.) through customization points
- **Exception Safety**: Robust error handling with automatic resource cleanup and strong exception guarantees
- **Modern C++20 Design**: Extensive use of concepts, three-way comparison, sender/receiver patterns, and contemporary C++ features

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
#include <io.hpp>
#include <netinet/in.h>

// Create a RAII socket handle
io::socket::socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Create socket address using the new socket_option-based API
struct sockaddr_in native_addr{};
native_addr.sin_family = AF_INET;
native_addr.sin_addr.s_addr = INADDR_ANY;
native_addr.sin_port = 0;  // Let system choose port

// Create a socket_address wrapper
auto server_addr = io::socket::make_address(&native_addr);

// Bind using the socket_address wrapper
io::bind(server_socket, server_addr);

// Start listening
io::listen(server_socket, 5);

// Accept incoming connections - high-level API returns managed objects
auto [client_socket, client_address] = io::accept(server_socket);
// Both client_socket and client_address are automatically managed
// Access client address data: *client_address contains the sockaddr_storage
// Access as specific type: reinterpret_cast<const sockaddr_in*>(&(*client_address))
```

### Client Socket Connection

```cpp
#include <io.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

// Create client socket
io::socket::socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Create server address using socket_address wrapper
struct sockaddr_in native_server_addr{};
native_server_addr.sin_family = AF_INET;
native_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
native_server_addr.sin_port = htons(8080);

auto server_addr = io::socket::make_address(&native_server_addr);

// Connect using socket_address
int result = io::connect(client_socket, server_addr);

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

### Advanced Message I/O

```cpp
#include <io.hpp>
#include <netinet/in.h>

// Create socket and connect
io::socket::socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// ... connection setup ...

// Create a socket message for scatter-gather I/O
io::socket::socket_message msg;

// Add multiple data buffers using thread-safe methods
std::string header = "HTTP/1.1 200 OK\r\n";
std::string content = "Hello, World!";
msg.push(header.data(), header.size());  // Thread-safe append
msg.push(content.data(), content.size());

// Alternative: Use emplace for in-place construction
msg.emplace("Connection: close\r\n\r\n");

// Send using scatter-gather I/O
auto bytes_sent = io::sendmsg(client_socket, msg.native(), MSG_NOSIGNAL);
if (bytes_sent > 0) {
    std::cout << "Sent " << bytes_sent << " bytes\n";
}

// Receive into message buffer
io::socket::socket_message recv_msg;
recv_msg.reserve_buffer(1024);  // Pre-allocate receive buffer
auto bytes_received = io::recvmsg(client_socket, recv_msg.native(), 0);
```

### Socket Address Management

```cpp
#include <io.hpp>
#include <netinet/in.h>

// Create a socket address wrapper using make_address
struct sockaddr_in native_addr{};
native_addr.sin_family = AF_INET;
native_addr.sin_addr.s_addr = INADDR_ANY;
native_addr.sin_port = htons(8080);

// Wrap in platform-independent socket_address
auto addr = io::socket::make_address(&native_addr);

// Use with socket operations - socket_address works directly with tag-invoke operations
io::socket::socket_handle socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
io::bind(socket, addr);

// Access underlying data when needed
// Access as sockaddr_storage: *addr
// Access as specific type: reinterpret_cast<const sockaddr_in*>(&(*addr))

// Copy and move semantics work as expected
auto addr_copy = addr;  // Deep copy
auto addr_moved = std::move(addr);  // Move operation
```

### High-Level Socket Operations

The library provides high-level operations that return managed objects for easier resource management:

```cpp
#include <io.hpp>
#include <netinet/in.h>

// Server setup
io::socket::socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Create socket address - two ways:

// Method 1: Create from existing sockaddr structure
struct sockaddr_in native_addr{};
native_addr.sin_family = AF_INET;
native_addr.sin_addr.s_addr = INADDR_ANY;
native_addr.sin_port = htons(80);
auto server_addr = io::socket::make_address(&native_addr);

// Method 2: Create empty socket_address and configure
auto server_addr2 = io::socket::make_address<struct sockaddr_in>();
auto *bind_addr = reinterpret_cast<struct sockaddr_in*>(&(*server_addr2));
bind_addr->sin_family = AF_INET;
bind_addr->sin_addr.s_addr = INADDR_ANY;
bind_addr->sin_port = htons(80);

io::bind(server_socket, server_addr);
io::listen(server_socket, 5);

// Accept using high-level API that returns managed objects
try {
    auto [client_socket, client_addr] = io::accept(server_socket);

    // client_socket is a fully managed socket_handle
    // client_addr is a socket_address with the client's address information

    const auto* addr_info = reinterpret_cast<const sockaddr_in*>(&(*client_addr));
    std::cout << "Client connected from port: " << ntohs(addr_info->sin_port) << std::endl;

    // Both objects are automatically cleaned up when they go out of scope
} catch (const std::system_error& e) {
    std::cerr << "Accept failed: " << e.what() << std::endl;
}
```

## Architecture

### Core Components

- **`io::socket::socket_handle`**: Thread-safe RAII socket wrapper with atomic storage, mutex-protected operations, and persistent error tracking for asynchronous operations
- **`io::socket::socket_address`**: Platform-independent socket address abstraction with safe data access via `data()` and `size()` methods
- **`io::socket::socket_message`**: Thread-safe message container supporting scatter-gather I/O, ancillary data, and advanced messaging patterns with efficient `push()` and `emplace()` methods for data management
- **Execution Framework**: Asynchronous execution system with executor, poll multiplexer, and I/O triggers supporting sender/receiver patterns with shared_ptr-based socket lifetime management, implemented in `src/io/execution/` with modular design
- **Tag-Dispatched Operations**: Extensible socket operations (`io::bind`, `io::connect`, `io::accept`, `io::sendmsg`, `io::recvmsg`, etc.) with multiple overloads using the `tag_invoke` pattern
- **Cross-Platform Abstraction**: Platform-specific implementations in `src/io/socket/platforms/` with unified interfaces
- **Error Handling**: Structured exception handling with `std::system_error` for high-level operations, return codes for low-level operations, and persistent error state tracking in socket handles

### Design Principles

- **Tag-Dispatched Customization**: Type-safe, extensible operations using `tag_invoke` pattern for compile-time dispatch
- **Cross-Platform Compatibility**: Unified API with platform-specific implementations using conditional compilation
- **RAII Resource Management**: Automatic socket cleanup with exception-safe constructors and destructors
- **Thread Safety**: Atomic socket storage with mutex protection for modification operations and shared_ptr-based lifetime management for asynchronous operations
- **Move-Only Semantics**: Clear resource ownership preventing accidental copies and resource leaks
- **Exception Safety**: Robust error handling with proper cleanup guaranteed in all failure scenarios
- **Asynchronous Execution**: Sender/receiver pattern implementation for scalable, non-blocking I/O operations
- **Modern C++20**: Extensive use of three-way comparison, concepts, sender/receiver patterns, and contemporary language features

### Performance Characteristics

- **Zero-Copy Operations**: Scatter-gather I/O support minimizes memory copying for high-performance applications
- **Lock-Free Reads**: Socket handle state queries use atomic operations for minimal overhead
- **Efficient Threading**: Mixed threading model with mutex protection only where necessary
- **Scalable I/O**: Poll multiplexer with sender/receiver patterns for handling large numbers of concurrent connections
- **Platform Optimization**: Native platform APIs used directly with minimal abstraction overhead
- **Memory Management**: Move-only semantics and RAII prevent unnecessary allocations and copies

### Dependencies

- **GoogleTest**: Auto-fetched via CMake FetchContent for unit testing
- **NVIDIA stdexec**: Auto-fetched via CPM for sender/receiver execution patterns
- **Boost.Predef**: Used for cross-platform compiler and OS detection (header-only)

### Code Quality

The project uses comprehensive static analysis with clang-tidy:

```bash
# Run clang-tidy on the entire codebase
clang-tidy src/**/*.cpp src/**/*.hpp -- -std=c++20 -I src/
```

Configured rules include `bugprone-*`, `cert-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, and `readability-*` checks (excludes `readability-braces-around-statements`, `readability-magic-numbers`, and `readability-implicit-bool-conversion` for flexibility).

### Thread Safety

- **Socket Handles**: Full thread safety with atomic reads and mutex-protected modifications
- **Socket Messages**: Thread-safe access to message data and buffers
- **Socket Addresses**: Thread-safe copy/move operations with proper synchronization
- **Tag-Invoke Operations**: Concurrent operation dispatch with no shared mutable state

## License

This project is licensed under the Apache License, Version 2.0. You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
