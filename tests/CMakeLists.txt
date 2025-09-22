include(GoogleTest)

set(TEST_NAMES
  socket_handle_test
  socket_address_test
  poll_triggers_test
  socket_option_test
  socket_message_test
  socket_dialog_test
  mock_poll_test
  mock_fcntl_test
  mock_accept_test
  mock_recvmsg_test
  mock_sendmsg_test
)

foreach(TEST_NAME IN LISTS TEST_NAMES)
  add_executable(
    ${TEST_NAME}
    ${TEST_NAME}.cpp
  )

  target_link_libraries(
    ${TEST_NAME}
    PRIVATE
    GTest::gtest_main
    io_static
  )

  target_include_directories(${TEST_NAME}
    PRIVATE
    "${INCLUDE_DIRS}"
    ${CMAKE_BINARY_DIR}/_deps/stdexec-src/include
  )

  if(IO_ENABLE_COVERAGE)
    target_link_libraries(${TEST_NAME} PRIVATE gcov)
  endif()

  gtest_discover_tests(${TEST_NAME})
endforeach()
