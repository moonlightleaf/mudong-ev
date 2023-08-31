add_executable(test_Logger test_Logger.cc)
target_link_libraries(test_Logger mudong-ev)

set(TEST_DIR ${EXECUTABLE_OUTPUT_PATH})
add_test(test_Logger ${TEST_DIR}/test_Logger)