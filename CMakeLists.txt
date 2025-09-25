cmake_minimum_required(VERSION 3.28)
project(async-berkeley)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# include(CPack) already included by stdexec

# Get CPM
# For more information on how to add CPM to your project, see: https://github.com/cpm-cmake/CPM.cmake#adding-cpm
include(cmake/CPM.cmake)

# include NVIDIA stdexec
CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main # This will always pull the latest code from the `main` branch. You may also use a specific release version or tag
)

# Include directories for build and distribution.
set(INCLUDE_DIRS
  "${CMAKE_SOURCE_DIR}/include/"
)

# Enable testing by default if this is a top-level project or
# in submodules if the project has explicitly set BUILD_TESTING
# by including CTest.
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) OR BUILD_TESTING)
    set(IO_BUILD_TESTING_DEFAULT ON)
else()
    set(IO_BUILD_TESTING_DEFAULT OFF)
endif()
option(IO_BUILD_TESTING "Enable testing." ${IO_BUILD_TESTING_DEFAULT})
if (IO_BUILD_TESTING)
  # Enable and configure code coverage
  option(IO_ENABLE_COVERAGE "Enable code coverage reporting with gcovr" ON)
  if (IO_ENABLE_COVERAGE)
    include(cmake/EnableCoverage.cmake)
  endif()

  # Enable tests.
  include(CTest)
  include(cmake/EnableTests.cmake)
endif()

# Configure benchmarks with googlebenchmark.
option(IO_ENABLE_BENCHMARKS "Enable benchmarking with google benchmark." OFF)
if (IO_ENABLE_BENCHMARKS)
  include(cmake/EnableBenchmarks.cmake)
endif()

# enable and configure doxygen documentation
option(IO_ENABLE_DOCS "Build Doxygen documentation" OFF)
if (IO_ENABLE_DOCS)
  include(cmake/EnableDocs.cmake)
endif()

# Add examples
option(IO_BUILD_EXAMPLES "Build example programs" ON)
if(IO_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Add the uninstall target.
include(cmake/Uninstall.cmake)
