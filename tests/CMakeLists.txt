set(TEST_NAMES
  socket_message_test
  socket_buffer_test
  socket_handle_test
  socket_address_test
)

include(GoogleTest)

foreach(TEST_NAME IN LISTS TEST_NAMES)
  add_executable(
    ${TEST_NAME}
    ${TEST_NAME}.cpp
  )

  target_link_libraries(
    ${TEST_NAME}
    PRIVATE
    GTest::gtest_main
    iosched
  )

  if(IOSCHED_ENABLE_COVERAGE)
    target_link_libraries(${TEST_NAME} PRIVATE gcov)
  endif()

  gtest_discover_tests(${TEST_NAME})
endforeach()
