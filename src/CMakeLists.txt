add_library(iosched
  buffers/socket_buffer.cpp
  socket/socket_handle.cpp
)

target_include_directories(iosched
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
