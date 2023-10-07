add_executable(echo_client EchoClient.cc)
target_link_libraries(echo_client mudong-ev)

add_executable(echo_server EchoServer.cc)
target_link_libraries(echo_server mudong-ev)

add_executable(addOne_server AddOneServer.cc)
target_link_libraries(addOne_server mudong-ev)

add_executable(addOne_client AddOneClient.cc)
target_link_libraries(addOne_client mudong-ev)