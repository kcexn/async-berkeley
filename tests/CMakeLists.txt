add_executable(
  socket_message_test
  socket_message_test.cpp
)

target_link_libraries(
  socket_message_test
  PRIVATE
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(socket_message_test)
