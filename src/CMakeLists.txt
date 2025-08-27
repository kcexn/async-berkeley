add_library(io
  io/socket/socket_handle.cpp
  io/socket/socket_address.cpp
  io/socket/socket_message.cpp
  io/socket/socket.cpp
  io/execution/poll_multiplexer.cpp
  io/execution/triggers.cpp
)

target_include_directories(io
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
)
