# EnableCoverage.cmake - Configure code coverage reporting for iosched project
#
# This module configures gcovr-based code coverage when IOSCHED_ENABLE_COVERAGE is enabled.
# It handles coverage compiler flags, linker settings, and creates coverage targets.
message(STATUS "Configuring code coverage with gcovr")

# Warn if not using Debug build type
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Coverage is typically used with Debug builds. Current build type: ${CMAKE_BUILD_TYPE}")
endif()

# Check if gcovr is available
find_program(GCOVR_EXECUTABLE gcovr)
if(NOT GCOVR_EXECUTABLE)
    message(FATAL_ERROR "gcovr not found. Please install gcovr: pip install gcovr")
endif()

# Add coverage flags
set(COVERAGE_FLAGS "--coverage -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

# Create coverage target
add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      --root ${CMAKE_SOURCE_DIR}
      --filter \"${CMAKE_SOURCE_DIR}/\(src/|include/\)\"
      --exclude-lines-by-pattern ".*= default.*"
      --html --html-details
      --output ${CMAKE_BINARY_DIR}/coverage/index.html
      --print-summary
      ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report with gcovr"
    DEPENDS ${TEST_NAMES}
)

# Create coverage-xml target for CI/CD integration
add_custom_target(coverage-xml
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      --root ${CMAKE_SOURCE_DIR}
      --filter \"${CMAKE_SOURCE_DIR}/\(src/|include/\)\"
      --exclude-lines-by-pattern ".*= default.*"
      --xml
      --output ${CMAKE_BINARY_DIR}/coverage/coverage.xml
      --print-summary
      ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage XML report with gcovr"
    DEPENDS ${TEST_NAMES}
)

message(STATUS "Code coverage enabled. Use 'cmake --build . --target coverage' to generate reports.")
