add_library(iosched
  socket/socket_handle.cpp
  socket/socket_address.cpp
  socket/socket_message.cpp
  socket/socket.cpp
)

target_include_directories(iosched
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
