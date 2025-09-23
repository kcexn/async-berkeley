# EnableTests.cmake - Configure GoogleTest for AsyncBerkeley project
#
# This module configures GoogleTest when IO_ENABLE_TESTS is enabled.
# It handles fetching GoogleTest, setting up testing, and adding the tests subdirectory.
message(STATUS "Configuring benchmarks with Google Benchmark")

# Fetch GoogleTest using FetchContent
include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.zip
)

# Make GoogleTest available
FetchContent_MakeAvailable(googlebenchmark)

# Add the tests subdirectory
add_subdirectory(benchmarks)

message(STATUS "GoogleBenchmark configured successfully")
