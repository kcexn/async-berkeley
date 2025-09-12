# AsyncBerkeley
[![Build](https://github.com/kcexn/async-berkeley/actions/workflows/build.yml/badge.svg)](https://github.com/kcexn/async-berkeley/actions/workflows/build.yml)
[![Tests](https://github.com/kcexn/async-berkeley/actions/workflows/tests.yml/badge.svg)](https://github.com/kcexn/async-berkeley/actions/workflows/tests.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/async-berkeley/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d2dfc8d21d4342f5915f18237628ac7f)](https://app.codacy.com/gh/kcexn/async-berkeley/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

AsyncBerkeley is a modern C++ socket I/O library that provides an asynchronous implementation of (most of) the
Berkeley Sockets API. It is built using NVIDIA's [stdexec library](https://github.com/NVIDIA/stdexec) and aims
to drop `stdexec` as a dependency once there is adequate compiler support for C++26's `std::execution`.
AsyncBerkeley aims to make asynchronous socket I/O as simple as the ubiquitous Berkeley sockets API.

## Features
- **Asynchronous Execution Framework**: Sender/receiver
pattern implementation with a simple poll based multiplexer
for scalable I/O operations.
- **Familiar Berkeley Sockets API**: Socket operations
include: `accept`, `bind`, `connect`, `fcntl`, `getpeername`,
`getsockname`, `getsockopt`, `listen`, `recvmsg`, `sendmsg`,
`setsockopt`, and `shutdown` with plans to add support for
`send`, `sendto`, `recv`, and `recvfrom`.
- **Full Support for Asynchronous Scatter/Gather Socket I/O.**:
`sendmsg` and `recvmsg` API support scatter/gather I/O using
the native `iovec`, and `msghdr` buffer structures.
- **Convenient socket helper classes for RAII**: Helpers like
`socket_dialog`, `socket_handle`, `socket_option`, and
`socket_address` provide RAII and a convenient C++20 `std::span` based API
for working with native sockets and socket addresses.

## Contributing
All contributions are welcome! AsyncBerkeley aims to be a library where
people can learn network programming, as well as C++'s `std::execution`.
Please open a Github issue if you have a question or send me an
[email](mailto:kevin.exton@pm.me).

## Quick Start
### Prerequisites
- CMake 3.28+
- C++20 compatible compiler
- **GoogleTest**: Auto-fetched via CMake FetchContent
for unit testing
- **NVIDIA stdexec**: Auto-fetched via CPM for
sender/receiver execution patterns

### Build and Install with Cmake
```bash
# Clone the repository
git clone https://github.com/kcexn/async-berkeley.git
cd async-berkeley

# Build with cmake.
cmake --preset release
cmake --build build/release

# Install with cmake.
cmake --install build/release

# Uninstall with cmake
cmake --build build/release -t uninstall
```

### Detailed Build Instructions
For comprehensive build instructions, dependency installation guides, code coverage setup, and troubleshooting, see [DEVELOPER.md](DEVELOPER.md).

### Documentation
API documentation is available at: [https://kcexn.github.io/async-berkeley/](https://kcexn.github.io/async-berkeley/)

To build documentation locally:
```bash
cmake --preset release -DIO_ENABLE_DOCS=ON
# Generate docs in build/release/docs
cmake --build build/release --t docs
# Deploy docs to top-level docs directory.
cmake --build build/release --t docs-deploy
# Open docs/html/index.hml in a browser.
```

## Usage
### Basic Socket Operations
```cpp
#include <io.hpp>
#include <netinet/in.h>

// Create a RAII socket handle
io::socket::socket_handle server_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Create socket address using the new socket_option-based API
auto server_addr = io::socket::make_address<sockaddr_in>();
server_addr->sin_family = AF_INET;
server_addr->sin_addr.s_addr = INADDR_ANY;
server_addr->sin_port = 0; // Let system choose port

// Bind using the socket_address wrapper
int err = io::bind(server_socket, server_addr);


// Start listening
err = io::listen(server_socket, 5);

auto client_address = io::socket::make_address<sockaddr_in>();
// Accept incoming connections - high-level API returns managed objects
auto [client_socket, addr] = io::accept(server_socket, client_address);

// Some socket types (like UNIX domain sockets) may return
// a differently sized address from accept. In this
// case, we will need to assign the returned addr back
// to client_address.
if(client_address != addr)
  client_address = addr;
```

### Client Socket Connection
```cpp
#include <io.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

// Create client socket
io::socket::socket_handle client_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

// Create server address from a native sockaddr structure.
struct sockaddr_in native_addr{};
native_addr.sin_family = AF_INET;
native_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
native_addr.sin_port = htons(8080);

auto server_addr = io::socket::make_address(&native_addr);

// Connect using socket_address
int result = io::connect(client_socket, server_addr);

if (result == 0) {
  // Connected successfully - send/receive data
  const char *message = "Hello, server!";

  io::socket::socket_message msg;
  msg.buffers.emplace_back(message, strlen(message));

  io::sendmsg(client_socket, msg, 0);
}
```

### Asynchronous Socket Operations
The library provides full asynchronous socket operations using the sender/receiver pattern with `stdexec`. Here is a snippet based on the ping/pong example in
`examples/`:

```cpp
class AsyncClient {
public:
  AsyncClient(const socket_address<sockaddr_in>& server_addr)
    : server_(server_addr), triggers_() {}

  void run() {
    // Create client socket dialog using triggers
    auto client = triggers_.emplace(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Connect asynchronously - returns a sender
    auto connect = io::connect(client, server_) |
                          then([this, client](const auto& connect_result) {
                              std::cout << "Connected to server\n";
                              // Start async message exchange
                              send_message(client);
                          }) |
                          upon_error([](const auto &error){
                            std::cerr << "Connection failed\n" << std::flush;
                          });

    // Start the async operation
    scope_.spawn(std::move(connect));

    // Event loop - wait for async operations to complete
    while (!done_) {
      triggers_.wait();
    }
  }

private:
  void send_message(const socket_dialog<poll_multiplexer>& client) {
    const char* message = "Hello from async client!\n";
    socket_message msg{};
    msg.buffers.emplace_back(const_cast<char*>(message), strlen(message));

    // Send message asynchronously
    auto sendmsg = io::sendmsg(client, msg, 0) |
                       then([this, client](std::streamsize bytes_sent) {
                             std::cout << "Sent " << bytes_sent << " bytes\n";
                             // Continue with receiving response
                             receive_response(client);
                           }) |
                       upon_error([](const auto& error) {
                        std::cerr << "Send failed\n";
                       });

    scope_.spawn(std::move(sendmsg));
  }

  void receive_response(const socket_dialog<poll_multiplexer>& client) {
    static std::array<char, 256> buffer;
    response_msg_.buffers.clear();
    response_msg_.buffers.emplace_back(buffer.data(), buffer.size());

    // Receive response asynchronously
    auto &buf = response_msg_.buffers.front();
    auto recvmsg = io::recvmsg(client, response_msg_, 0) |
                   then([this, buf](auto bytes_received){
                     std::string response(static_cast<char*>(buf.iov_base),
                                          bytes_received);
                     std::cout << "Received: " << response;
                     done_ = true;  // Mark completion
                   }) |
                   upon_error([](const auto &error){
                     std::cerr << "Receive failed\n";
                     done_ = true;
                   });

    scope_.spawn(std::move(recvmsg));
  }

  socket_address<sockaddr_in> server_;
  basic_triggers<poll_multiplexer> triggers_;
  async_scope scope_;
  socket_message response_msg_;
  std::atomic<bool> done_{false};
};
```
## Limitations
- **Windows Support**: Windows support is currently planned but very far
from complete.
- **I/O Multiplexer Support**: Only the `poll` multiplexer is currently
supported. There are plans to support `io_uring`, `kqueue`, and
Windows' `IOCP` at some stage in the future.
- **Integration point support**: Currently the I/O multiplexer can
only be used with `socket_handle`'s or `socket_dialog`'s. Support
for arbitrary file descriptors so that third-party libraries can be
integrated into the I/O loop is planned for some stage in the future.
- **Complete Berkeley Sockets API support**: `recv`, `recvfrom`, `send`,
and `sendto` are not yet supported.
