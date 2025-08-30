# DEVELOPER.md

Developer setup guide for the iosched C++20 I/O scheduling library with asynchronous execution framework.

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

### Installing Dependencies

#### Boost Libraries (Required)

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libboost-dev
# For complete Boost installation:
sudo apt-get install libboost-all-dev
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
# Install vcpkg first, then:
vcpkg install boost:x64-windows
```

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
# Should show: src/, tests/, CMakeLists.txt, CMakePresets.json, etc.
```

### 2. Verify Prerequisites

```bash
# Check CMake version (should be 3.26+)
cmake --version

# Check C++20 compiler support
g++ --version  # or clang++ --version

# Check if Boost is installed
pkg-config --modversion boost
# or
find /usr -name "boost" -type d 2>/dev/null | head -5
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
./build/debug/tests/socket_test
./build/debug/tests/poll_triggers_test  # Execution framework tests
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

#### Build with Tests Enabled

```bash
mkdir build && cd build

# Configure with tests
cmake .. -DIOSCHED_ENABLE_TESTS=ON

# Build everything including tests
cmake --build .

# Run tests
ctest
# or manually:
./tests/socket_handle_test
./tests/socket_message_test
./tests/socket_address_test
./tests/socket_test
```

#### Build with Tests and Coverage

```bash
mkdir build && cd build

# Configure with tests and coverage
cmake .. -DIOSCHED_ENABLE_TESTS=ON -DIOSCHED_ENABLE_COVERAGE=ON

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
cmake .. -DIOSCHED_ENABLE_DOCS=ON

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
cmake .. -DIOSCHED_ENABLE_DOCS=ON

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
cmake .. -DIOSCHED_ENABLE_TESTS=ON -DIOSCHED_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

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
- `src/` directory files only
- All production code (excludes test files)
- Line coverage and branch coverage metrics

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

# Run with specific test filter (if using Google Test)
./build/debug/tests/socket_handle_test --gtest_filter="*ConstructorTest*"
```

### Test Categories

Current test suites:
- **socket_handle_test**: Tests for RAII socket wrapper (`io::socket::socket_handle`)
- **socket_message_test**: Tests for thread-safe socket messages (`io::socket::socket_message`) with push/emplace functionality
- **socket_address_test**: Tests for platform-independent socket address abstraction (`io::socket::socket_address`)
- **socket_test**: Tests for cross-platform socket operations and tag-dispatched customization points
- **poll_triggers_test**: Tests for asynchronous execution framework including executor, poll multiplexer, and I/O triggers with sender/receiver patterns

### High Test Coverage

The project maintains **98% code coverage** through comprehensive testing including:
- Success and failure scenarios for all operations
- Thread safety and concurrent access testing
- Resource management and RAII verification
- Cross-platform compatibility testing
- Edge cases and error condition handling

## Troubleshooting

### Common Build Issues

#### CMake Cannot Find Boost

```bash
# Specify Boost location manually
cmake .. -DBOOST_ROOT=/usr/local -DBoost_NO_SYSTEM_PATHS=TRUE

# For custom installations
cmake .. -DBOOST_INCLUDEDIR=/path/to/boost/include -DBOOST_LIBRARYDIR=/path/to/boost/lib

# Windows with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

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

Edit source files in `src/` directory, add tests in `tests/` directory.

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
clang-tidy src/**/*.cpp src/**/*.hpp -- -std=c++20 -I src/

# Format code (if clang-format is configured)
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

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
