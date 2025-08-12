cmake_minimum_required(VERSION 3.26)
project(iosched)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include boost
find_package(Boost REQUIRED)

# enable tests with googletest
option(IOSCHED_ENABLE_TESTS "Build the tests for this project" OFF)
# enable code coverage
option(IOSCHED_ENABLE_COVERAGE "Enable code coverage reporting with gcovr" OFF)

# configure tests
if(IOSCHED_ENABLE_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_subdirectory(tests)
endif()

# configure coverage reporting
if(IOSCHED_ENABLE_COVERAGE)
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Coverage is typically used with Debug builds. Current build type: ${CMAKE_BUILD_TYPE}")
  endif()

  # Check if gcovr is available
  find_program(GCOVR_EXECUTABLE gcovr)
  if(NOT GCOVR_EXECUTABLE)
    message(FATAL_ERROR "gcovr not found. Please install gcovr: pip install gcovr")
  endif()

  # Add coverage flags
  set(COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

  # Create coverage target
  add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      --root ${CMAKE_SOURCE_DIR}
      --filter ${CMAKE_SOURCE_DIR}/src/
      --html --html-details
      --output ${CMAKE_BINARY_DIR}/coverage/index.html
      --print-summary
      ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report with gcovr"
    DEPENDS socket_message_test socket_buffer_test
  )

  # Create coverage-xml target for CI/CD integration
  add_custom_target(coverage-xml
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      --root ${CMAKE_SOURCE_DIR}
      --filter ${CMAKE_SOURCE_DIR}/src/
      --xml
      --output ${CMAKE_BINARY_DIR}/coverage/coverage.xml
      --print-summary
      ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage XML report with gcovr"
    DEPENDS socket_message_test socket_buffer_test
  )

  message(STATUS "Code coverage enabled. Use 'cmake --build . --target coverage' to generate reports.")
endif()

add_subdirectory(src)
