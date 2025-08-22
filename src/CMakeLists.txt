add_library(iosched
  socket/socket_handle.cpp
  socket/socket_address.cpp
  socket/socket_message.cpp
  socket/socket.cpp
  execution/poll_multiplexer.cpp
)

target_include_directories(iosched
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)
