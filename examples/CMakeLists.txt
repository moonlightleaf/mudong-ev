add_executable(echo_client EchoClient.cc)
target_link_libraries(echo_client mudong-ev)

add_executable(echo_server EchoServer.cc)
target_link_libraries(echo_server mudong-ev)