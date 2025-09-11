cmake_minimum_required(VERSION 3.28)
project(iosched)

# iosched requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
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
  "${CMAKE_SOURCE_DIR}/vendor/"
)

# enable and configure code coverage
option(IO_ENABLE_COVERAGE "Enable code coverage reporting with gcovr" OFF)
if (IO_ENABLE_COVERAGE)
  include(cmake/EnableCoverage.cmake)
endif()

# Configure tests with googletest
if (BUILD_TESTING)
  include(cmake/EnableTests.cmake)
endif()

# enable and configure doxygen documentation
option(IO_ENABLE_DOCS "Build Doxygen documentation" OFF)
if (IO_ENABLE_DOCS)
  include(cmake/EnableDocs.cmake)
endif()

# Add the static and shared libraries to the build and installation list.
add_subdirectory(src)

# Add examples
option(IO_BUILD_EXAMPLES "Build example programs" ON)
if(IO_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Add the uninstall target.
include(cmake/Uninstall.cmake)
