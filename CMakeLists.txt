cmake_minimum_required(VERSION 3.28)
project(
  AsyncBerk
  VERSION 0.3.3
  LANGUAGES CXX)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Get CPM For more information on how to add CPM to your project, see:
# https://github.com/cpm-cmake/CPM.cmake#adding-cpm
include(cmake/CPM.cmake)

# include NVIDIA stdexec
CPMAddPackage(
  URI "gh:NVIDIA/stdexec#main"
  OPTIONS
    "STDEXEC_BUILD_EXAMPLES OFF"
    "STDEXEC_BUILD_TESTING OFF"
)

# Add AsyncBerkeley targets
add_library(asyncberk INTERFACE)
target_include_directories(
  asyncberk INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(asyncberk INTERFACE STDEXEC::stdexec)
add_library(${PROJECT_NAME}::asyncberk ALIAS asyncberk)

# Add AsyncBerkeley install targets
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Enable testing by default if this is a top-level project or in submodules if
# the project has explicitly set BUILD_TESTING by including CTest.
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) OR BUILD_TESTING)
  set(IO_BUILD_TESTING_DEFAULT ON)
else()
  set(IO_BUILD_TESTING_DEFAULT OFF)
endif()

option(IO_BUILD_TESTING "Enable testing." ${IO_BUILD_TESTING_DEFAULT})
if(IO_BUILD_TESTING)
  # Add GoogleTest
  message(STATUS "Configure tests with GoogleTest")
  CPMAddPackage(
    NAME googletest
    URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES)
  # For Windows: Prevent overriding the parent project's compiler/linker
  # settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Enable and configure code coverage
  option(IO_ENABLE_COVERAGE "Enable code coverage reporting with gcovr" ON)
  if(IO_ENABLE_COVERAGE)
    include(cmake/EnableCoverage.cmake)
  endif()

  # Enable tests.
  include(CTest)
  add_subdirectory(tests)

  message(STATUS "GoogleTest configured successfully")
endif()

# Configure benchmarks with googlebenchmark.
option(IO_ENABLE_BENCHMARKS "Enable benchmarking with google benchmark." OFF)
if(IO_ENABLE_BENCHMARKS)
  CPMAddPackage(
    NAME googlebenchmark
    URL "https://github.com/google/benchmark/archive/refs/tags/v1.9.4.zip"
    EXCLUDE_FROM_ALL YES
    SYSTEM YES)

  add_subdirectory(benchmarks)
endif()

# enable and configure doxygen documentation
option(IO_ENABLE_DOCS "Build Doxygen documentation" OFF)
if(IO_ENABLE_DOCS)
  include(cmake/EnableDocs.cmake)
endif()

# Add examples
option(IO_BUILD_EXAMPLES "Build example programs" ON)
if(IO_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
