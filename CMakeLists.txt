cmake_minimum_required(VERSION 3.26)
project(iosched)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
