set(SRC_FILES
        Logger.hpp
        noncopyable.hpp
        EventLoop.cc EventLoop.hpp
        EPollPoller.cc EPollPoller.hpp
        Channel.cc Channel.hpp
        Acceptor.cc Acceptor.hpp
        Buffer.cc Buffer.hpp
        Callbacks.hpp
        InetAddress.cc InetAddress.hpp
        TcpConnection.cc TcpConnection.hpp
        TcpServerSingle.cc TcpServerSingle.hpp
        TcpServer.cc TcpServer.hpp
        ThreadPool.cc ThreadPool.hpp
        Connector.cc Connector.hpp
        TcpClient.cc TcpClient.hpp
        CountDownLatch.hpp
        EventLoopThread.cc EventLoopThread.hpp
        TimerQueue.cc TimerQueue.hpp
        Timer.hpp
        Timestamp.hpp
)

add_library(mudong-ev STATIC ${SRC_FILES})
target_link_libraries(mudong-ev pthread)

install(TARGETS mudong-ev DESTINATION lib)
set(HEADERS
        Acceptor.hpp
        Buffer.hpp
        Callbacks.hpp
        Channel.hpp
        Connector.hpp
        CountDownLatch.hpp
        EPollPoller.hpp
        EventLoop.hpp
        EventLoopThread.hpp
        InetAddress.hpp
        Logger.hpp
        noncopyable.hpp
        TcpClient.hpp
        TcpConnection.hpp
        TcpServer.hpp
        TcpServerSingle.hpp
        ThreadPool.hpp
        Timer.hpp
        TimerQueue.hpp
        Timestamp.hpp
        ThreadPool.hpp
)


# set_target_properties(mudong-ev PROPERTIES LINKER_LANGUAGE CXX)
install(FILES ${HEADERS} DESTINATION "${CMAKE_BINARY_DIR}/include")

