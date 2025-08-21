cmake_minimum_required(VERSION 3.26)
project(iosched)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get CPM
# For more information on how to add CPM to your project, see: https://github.com/cpm-cmake/CPM.cmake#adding-cpm
include(cmake/CPM.cmake)

# include NVIDIA stdexec
CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main # This will always pull the latest code from the `main` branch. You may also use a specific release version or tag
)

# include boost
find_package(Boost REQUIRED)

# enable and configure tests with googletest
option(IOSCHED_ENABLE_TESTS "Build the tests for this project" OFF)
include(cmake/EnableTests.cmake)
# enable and configure code coverage
option(IOSCHED_ENABLE_COVERAGE "Enable code coverage reporting with gcovr" OFF)
include(cmake/EnableCoverage.cmake)
# enable and configure doxygen documentation
option(IOSCHED_ENABLE_DOCS "Build Doxygen documentation" OFF)
include(cmake/EnableDocs.cmake)

add_subdirectory(src)
