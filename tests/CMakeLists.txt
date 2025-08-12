add_executable(
  socket_message_test
  socket_message_test.cpp
)

target_link_libraries(
  socket_message_test
  PRIVATE
  GTest::gtest_main
  iosched
)

if(IOSCHED_ENABLE_COVERAGE)
  target_link_libraries(socket_message_test PRIVATE gcov)
endif()

add_executable(
  socket_buffer_test
  socket_buffer_test.cpp
)

target_link_libraries(
  socket_buffer_test
  PRIVATE
  GTest::gtest_main
  iosched
)

if(IOSCHED_ENABLE_COVERAGE)
  target_link_libraries(socket_buffer_test PRIVATE gcov)
endif()

include(GoogleTest)
gtest_discover_tests(socket_message_test)
gtest_discover_tests(socket_buffer_test)
