# DEVELOPER.md

Developer setup guide for the iosched C++20 I/O scheduling library with asynchronous execution framework, persistent socket error tracking, and sender/receiver patterns.

## Prerequisites

### System Requirements

- **Operating System**: Linux, macOS, or Windows
- **CMake**: Version 3.26 or higher
- **C++ Compiler**: C++20 compatible compiler
  - GCC 10+ (recommended for Linux)
  - Clang 12+ (recommended for macOS)
  - MSVC 19.29+ (Visual Studio 2019 16.10+) or MinGW-w64 for Windows
- **Build System**: Ninja (recommended) or Unix Makefiles
- **Git**: For repository cloning

### Project Dependencies

This project uses several modern C++ libraries and tools:

- **GoogleTest**: Auto-fetched via CMake FetchContent for comprehensive unit testing
- **NVIDIA stdexec**: Auto-fetched via CPM for sender/receiver execution patterns and asynchronous operations
- **CPM.cmake**: CMake package manager for automatic dependency fetching and management

### Installing Dependencies

#### Development Tools (Optional but Recommended)

**Documentation Generator:**
```bash
# Ubuntu/Debian
sudo apt-get install doxygen

# macOS
brew install doxygen

# Fedora
sudo dnf install doxygen

# Windows
# Download from https://www.doxygen.nl/download.html
```

**Code Coverage Tool:**
```bash
pip install gcovr
```

**Static Analysis:**
```bash
# Ubuntu/Debian
sudo apt-get install clang-tidy

# macOS
brew install llvm

# Fedora
sudo dnf install clang-tools-extra
```

**Build Tools:**
```bash
# Ubuntu/Debian
sudo apt-get install ninja-build cmake

# macOS
brew install ninja cmake

# Fedora
sudo dnf install ninja-build cmake
```

## Repository Setup

### 1. Clone the Repository

```bash
# Clone the main repository
git clone https://github.com/kcexn/iosched.git
cd iosched

# Verify the repository structure
ls -la
# Should show: include/, src/, tests/, CMakeLists.txt, CMakePresets.json, etc.
```

### 2. Verify Prerequisites

```bash
# Check CMake version (should be 3.26+)
cmake --version

# Check C++20 compiler support
g++ --version  # or clang++ --version
```

## Build Instructions

### Method 1: Using CMake Presets (Recommended)

#### Debug Build with Tests

```bash
# Configure debug build (enables tests and coverage)
cmake --preset debug

# Build the project
cmake --build --preset debug

# Run all tests
ctest --preset debug

# Run individual test executables
./build/debug/tests/socket_handle_test
./build/debug/tests/socket_message_test
./build/debug/tests/socket_address_test
./build/debug/tests/socket_option_test
./build/debug/tests/poll_triggers_test  # Execution framework tests

# Run example programs (if IO_BUILD_EXAMPLES=ON)
./build/debug/examples/async_ping_pong_client
```

#### Examples Build

The project includes comprehensive async networking examples demonstrating the sender/receiver pattern:

```bash
# Configure debug build with examples enabled (default: ON)
cmake --preset debug -DIO_BUILD_EXAMPLES=ON

# Build examples
cmake --build --preset debug
# In another terminal, run client (connects to 127.0.0.1:8080, sends 5 pings by default)
./build/debug/examples/async_ping_pong_client
```

**Example Programs:**
- **`async_ping_pong_client.cpp`**: Asynchronous client showcasing connection establishment, message sending/receiving, and proper async operation chaining with error handling
```

#### Release Build (Production)

```bash
# Configure release build (optimized, no tests)
cmake --preset release

# Build optimized version
cmake --build --preset release
```

#### Benchmark Build (High Performance)

```bash
# Configure benchmark build (maximum optimization)
cmake --preset benchmark

# Build with aggressive optimizations
cmake --build --preset benchmark

# Run benchmark tests
ctest --preset benchmark
```

### Method 2: Manual Build Configuration

#### Basic Build (No Tests)

```bash
mkdir build && cd build

# Configure basic build
cmake ..

# Build the library
cmake --build .
```

#### Build with Tests and Examples Enabled

```bash
mkdir build && cd build

# Configure with tests and examples
cmake .. -DIO_ENABLE_TESTS=ON -DIO_BUILD_EXAMPLES=ON

# Build everything including tests and examples
cmake --build .

# Run tests
ctest
# or manually:
./tests/socket_handle_test
./tests/socket_message_test
./tests/socket_address_test
./tests/socket_option_test
./tests/poll_triggers_test

# In another terminal:
./examples/async_ping_pong_client
```

#### Build with Tests and Coverage

```bash
mkdir build && cd build

# Configure with tests and coverage
cmake .. -DIO_ENABLE_TESTS=ON -DIO_ENABLE_COVERAGE=ON

# Build with coverage instrumentation
cmake --build .

# Run tests to generate coverage data
ctest

# Generate HTML coverage report
cmake --build . --target coverage
# View report: build/coverage/index.html

# Generate XML coverage report (for CI/CD)
cmake --build . --target coverage-xml
# Output: build/coverage/coverage.xml
```

#### Build with Documentation

```bash
mkdir build && cd build

# Configure with documentation enabled
cmake .. -DIO_ENABLE_DOCS=ON

# Build the documentation
cmake --build . --target docs

# View documentation: build/docs/html/index.html

# Deploy docs to GitHub Pages format
cmake --build . --target docs-deploy
# Copies to docs/html/ for GitHub Pages deployment
```

## Documentation Generation

### Prerequisites for Documentation

```bash
# Install Doxygen (if not already installed)
# Ubuntu/Debian
sudo apt-get install doxygen

# macOS
brew install doxygen

# Fedora
sudo dnf install doxygen

# Verify installation
doxygen --version
```

### Generating Documentation

#### Using Presets (Recommended)

```bash
# Configure debug build (includes documentation if enabled)
cmake --preset debug

# Documentation must be enabled during configure step, not build
# Reconfigure with documentation enabled:
# cmake --preset debug -DIOSCHED_ENABLE_DOCS=ON

# Generate documentation
cmake --build --preset debug --target docs

# Open documentation in browser
xdg-open build/debug/docs/html/index.html  # Linux
open build/debug/docs/html/index.html      # macOS
start build/debug/docs/html/index.html     # Windows
```

#### Manual Documentation Setup

```bash
mkdir build && cd build

# Configure with documentation enabled
cmake .. -DIO_ENABLE_DOCS=ON

# Generate API documentation
cmake --build . --target docs

# Deploy to GitHub Pages format
cmake --build . --target docs-deploy
```

### Documentation Output Locations

- **HTML Documentation**: `build/docs/html/index.html` or `build/debug/docs/html/index.html` (main documentation)
- **GitHub Pages**: `docs/html/index.html` (after running `docs-deploy` target)

### Documentation Features

The generated documentation includes:
- Complete API reference for all classes and functions
- Code examples and usage patterns
- Cross-referenced source code listings
- Search functionality
- README content as main page
- Class hierarchy diagrams (if Graphviz is installed)

### Customizing Documentation

Edit `docs/Doxyfile` to customize:
- Project information and branding
- Output formats and styling
- Input file filters
- Documentation sections

## Code Coverage

### Prerequisites for Coverage

```bash
# Install gcovr (if not already installed)
pip install gcovr

# Verify installation
gcovr --version
```

### Generating Coverage Reports

#### Using Presets (Recommended)

```bash
# Configure debug build (includes coverage)
cmake --preset debug

# Build with coverage instrumentation
cmake --build --preset debug

# Run tests to generate coverage data
ctest --preset debug

# Generate HTML coverage report
cmake --build --preset debug --target coverage

# Open coverage report in browser
xdg-open build/debug/coverage/index.html  # Linux
open build/debug/coverage/index.html      # macOS
start build/debug/coverage/index.html     # Windows
```

#### Manual Coverage Setup

```bash
mkdir build && cd build

# Configure with coverage enabled
cmake .. -DIO_ENABLE_TESTS=ON -DIO_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

# Build with coverage flags
cmake --build .

# Run all tests
ctest

# Generate coverage reports
cmake --build . --target coverage      # HTML report
cmake --build . --target coverage-xml  # XML report for CI
```

### Coverage Report Locations

- **HTML Report**: `build/coverage/index.html` or `build/debug/coverage/index.html` (interactive, detailed)
- **XML Report**: `build/coverage/coverage.xml` or `build/debug/coverage/coverage.xml` (for CI/CD integration)

### Coverage Targets

The coverage analysis focuses on:
- `src/` directory implementation files only (`.cpp` files)
- All production code (excludes test files and header-only templates)
- Line coverage and branch coverage metrics
- Template instantiations through comprehensive test coverage

## Running Tests

### All Tests

```bash
# Using presets
ctest --preset debug

# Manual approach
cd build
ctest

# Verbose test output
ctest --verbose

# Parallel test execution
ctest -j$(nproc)  # Linux/macOS
ctest -j%NUMBER_OF_PROCESSORS%  # Windows
```

### Individual Tests

```bash
# Run specific test by name
ctest --preset debug -R socket_handle_test

# Run test executable directly
./build/debug/tests/socket_handle_test
./build/debug/tests/socket_option_test  # For socket_option wrapper tests

# Run with specific test filter (if using Google Test)
./build/debug/tests/socket_handle_test --gtest_filter="*ConstructorTest*"
```

### Test Categories

Current test suites:
- **socket_handle_test**: Tests for RAII socket wrapper (`io::socket::socket_handle`) including persistent error tracking and comprehensive tag-invoke operations (both sync and async variants)
- **socket_message_test**: Tests for thread-safe socket messages (`io::socket::socket_message`) with push/emplace functionality and scatter-gather I/O support
- **socket_address_test**: Tests for platform-independent socket address abstraction (`io::socket::socket_address`) and `socket_option` wrapper with template-based construction
- **socket_option_test**: Tests for the generic `socket_option` wrapper template used by socket_address and other socket option types
- **poll_triggers_test**: Tests for asynchronous execution framework including executor, poll multiplexer, execution triggers, and socket dialog interface with sender/receiver patterns and shared_ptr-based lifetime management

### High Test Coverage

The project maintains **100% code coverage** through comprehensive testing including:
- Success and failure scenarios for all operations
- Thread safety and concurrent access testing
- Resource management and RAII verification
- Cross-platform compatibility testing
- Edge cases and error condition handling
- Persistent socket error tracking validation
- Asynchronous execution patterns with lifetime management
- Socket dialog interface testing for async operations
- Dual sync/async operation implementations

## Troubleshooting

### Common Build Issues

#### Coverage Tools Missing

```bash
# If gcovr is not found:
pip install --user gcovr
# or
pip install gcovr

# If clang-tidy is missing:
sudo apt-get install clang-tidy  # Ubuntu
brew install llvm               # macOS
```

#### Build System Issues

```bash
# If Ninja is not available, use Unix Makefiles
cmake .. -G "Unix Makefiles"

# For Windows without Ninja
cmake .. -G "Visual Studio 17 2022"  # or appropriate version
```

### Clean Build

```bash
# Remove build directory and start fresh
rm -rf build/
cmake --preset debug
cmake --build --preset debug
```

## Development Workflow

### 1. Setup Development Environment

```bash
git clone https://github.com/kcexn/iosched.git
cd iosched
cmake --preset debug
cmake --build --preset debug
```

### 2. Make Changes

Edit source files in `include/` directory for public headers (organized by component: `include/io/socket/`, `include/io/execution/`, `include/io/detail/`) and implementation files in `src/` directory, add tests in `tests/` directory.

**For Asynchronous Development:**
- Study the examples in `examples/async_ping_pong_*.cpp` for async patterns
- Use `basic_triggers<poll_multiplexer>` instead of manual executor management
- All async operations (`connect`, `accept`, `sendmsg`, `recvmsg`) return senders
- Chain operations with `stdexec::then()` and handle errors with `stdexec::upon_error()`
- Manage async lifetimes with `exec::async_scope` and `scope_.spawn()`

### 3. Build and Test

```bash
# Incremental build
cmake --build --preset debug

# Run tests
ctest --preset debug

# Check coverage
cmake --build --preset debug --target coverage

# Generate documentation (if working on API docs)
cmake --build --preset debug --target docs
```

### 4. Code Quality Checks

```bash
# Run static analysis
clang-tidy src/**/*.cpp include/**/*.hpp -- -std=c++20 -I include/

# Format code (if clang-format is configured)
find include src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### 5. Source Code Organization

The library follows a clean separation between public interface and implementation:

#### Public Interface (`include/` directory)
- **Socket Components**: Place socket-related headers in `include/io/socket/` (handles, addresses, messages, dialogs)
- **Execution Framework**: Add execution-related headers to `include/io/execution/` with utilities in `detail/` and implementations in `impl/`
- **Tag-Invoke CPOs**: Define customization point objects in `include/io/detail/customization.hpp`
- **Socket Operations**: Synchronous implementations in `include/io/socket/detail/sync_operations.hpp` and asynchronous versions in `include/io/socket/detail/async_operations.hpp`
- **Platform Support**: Add platform-specific headers to `include/io/socket/platforms/` or `include/io/detail/platforms/`
- **Main Header**: Export public API through `include/io.hpp`

#### Implementation (`src/` directory)
- **Core Implementations**: Only `.cpp` files containing actual implementations (currently: `poll_multiplexer.cpp`, `socket_handle.cpp`, `socket_message.cpp`)
- **New Implementations**: Add new `.cpp` files for components requiring implementation code

#### Tests (`tests/` directory)
- **Test Files**: Create corresponding test files following the existing naming pattern
- **Coverage**: Maintain 100% code coverage by testing all public API functionality

#### Examples (`examples/` directory)
- **Example Programs**: Demonstrate real-world usage patterns and best practices
- **Async Patterns**: `async_ping_pong_server.cpp` and `async_ping_pong_client.cpp` showcase asynchronous programming with `basic_triggers` and sender/receiver patterns
- **CMake Integration**: Examples are built automatically when `IO_BUILD_EXAMPLES=ON` (default)
- **Documentation**: Examples serve as living documentation and are referenced in README.md

When adding new features, follow the established structure and ensure proper separation between interface and implementation.

## IDE Integration

### VS Code

Create `.vscode/settings.json`:
```json
{
    "cmake.configurePreset": "debug",
    "cmake.buildPreset": "debug",
    "cmake.testPreset": "debug",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

### CLion

CLion automatically detects `CMakePresets.json` and provides preset selection in the IDE.

## Performance Testing

For performance testing, use the benchmark preset:

```bash
cmake --preset benchmark
cmake --build --preset benchmark

# Run performance-critical tests
ctest --preset benchmark

# Profile with perf (Linux)
perf record -g ./build/benchmark/tests/socket_handle_test
perf report
```
