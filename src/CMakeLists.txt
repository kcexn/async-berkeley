# Define the source and header files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

# Vendor the Boost.Predef headers.
option(IO_VENDOR_BOOST_PREDEF "Use the vendored version of Boost.Predef" ON)
if (IO_VENDOR_BOOST_PREDEF)
  file(GLOB_RECURSE VENDOR_BOOST_HEADERS
    "${CMAKE_SOURCE_DIR}/vendor/boost/*.h"
    "${CMAKE_SOURCE_DIR}/vendor/boost/*.hpp"
  )
  list(APPEND HEADERS ${VENDOR_BOOST_HEADERS})
endif()

# Dynamically linked library
add_library(io_shared SHARED ${SOURCES})
set_target_properties(io_shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(io_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(io_shared PROPERTIES OUTPUT_NAME "io")
target_include_directories(io_shared
  PUBLIC
  "${INCLUDE_DIRS}"
  ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)
target_sources(io_shared
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${INCLUDE_DIRS}"
  FILES ${HEADERS}
)

# Statically linked library
add_library(io_static STATIC ${SOURCES})
set_target_properties(io_static PROPERTIES OUTPUT_NAME "io")
target_include_directories(io_static
  PUBLIC
  "${INCLUDE_DIRS}"
  ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)
target_sources(io_static
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${INCLUDE_DIRS}"
  FILES ${HEADERS}
)

install(TARGETS io_shared io_static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  FILE_SET HEADERS DESTINATION include
  RUNTIME DESTINATION bin
)
