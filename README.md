# iosched

A high-performance C++17 I/O scheduling library providing asynchronous socket operations, event polling, and streaming interfaces with standard library integration.

## Features

- **Custom Socket Buffers**: `std::streambuf` implementation with 32KB minimum buffers and dynamic resizing
- **Event Polling**: Template-based polling system using Linux `poll(2)` with O(log n) binary search optimization
- **Stream Interface**: Full C++ `iostream` compatibility for socket operations
- **Non-blocking I/O**: Asynchronous socket operations using `MSG_DONTWAIT` flags
- **Memory Efficient**: RAII-based resource management with automatic vector shrinking
- **Policy-Based Design**: Template architecture for extensible polling and trigger systems

## Building

### Requirements

- CMake 3.26+
- C++17 compatible compiler
- Linux (uses Linux-specific socket APIs)

### Quick Start (CMake Presets - Recommended)

```bash
# Configure debug build with tests enabled
cmake --preset debug

# Build the project
cmake --build --preset debug

# Run tests
ctest --preset debug
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
```

### Running Tests

```bash
# Using presets
ctest --preset debug

# Manual approach
cd build
ctest
# Or run specific test:
./tests/hello_test
```

## Usage

### Basic Socket Stream

```cpp
#include "src/streams.hpp"
#include <iostream>

// Create a TCP socket stream
io::streams::sockstream sock(AF_INET, SOCK_STREAM, 0);

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

### Event Polling

```cpp
#include "src/io.hpp"
#include <chrono>

// Self-contained trigger with embedded poller
io::trigger triggers;

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

- **`io::buffers::sockbuf`**: Custom `std::streambuf` with dynamic 32KB+ buffers, handles TCP/UDP
- **`io::basic_poller<T>`**: Template-based event polling using Linux `poll(2)` 
- **`io::basic_trigger<T>`**: Event trigger management with O(log n) binary search on sorted handle lists
- **`io::streams::sockstream`**: Full `std::iostream` wrapper with stream operators
- **`io::basic_handler<T>`**: Event processing interface for building async applications

### Design Principles

- **Policy-Based Templates**: Core classes templated for extensible polling backends
- **Non-blocking by Design**: All socket operations use `MSG_DONTWAIT` flags
- **RAII Resource Management**: Automatic cleanup with `std::shared_ptr<socket_message>` buffers
- **Standard Library Integration**: Full compatibility with `std::iostream`, algorithms, and containers
- **Memory Optimization**: Automatic vector shrinking when capacity > 8x size and > 256 elements
- **Move-Only Semantics**: Non-copyable socket classes for clear resource ownership

### Dependencies

- **GoogleTest**: Auto-fetched via CMake FetchContent for unit testing
- **NVIDIA stdexec**: Included via CPM.cmake for future execution model support

## License

This project is licensed under the Apache License, Version 2.0. You may obtain a copy of the License at:

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
