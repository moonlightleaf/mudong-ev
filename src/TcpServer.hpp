#pragma once

#include <thread>
#include <condition_variable>

#include "TcpServerSingle.hpp"

namespace mudong {

namespace ev {

class TcpServerSingle;
class EventLoop;
class InetAddress;

class TcpServer : noncopyable {

public:
    TcpServer(EventLoop* loop, const InetAddress& local);
    ~TcpServer();

    // n <= 1，则运行在baseLoop thread中；否则，将会启动另外n - 1个EventLoopThread
    void setNumThread(size_t n);

    void start();

    void setThreadInitCallback(const ThreadInitCallback&);
    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);

private:
    void startInLoop();
    void runInThread(size_t index);

    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadPtrList = std::vector<ThreadPtr>;
    using TcpServerSinglePtr = std::unique_ptr<TcpServerSingle>;
    using EventLoopList = std::vector<EventLoop*>;

    EventLoop* baseLoop_;
    TcpServerSinglePtr baseServer_;
    ThreadPtrList threads_;
    EventLoopList eventLoops_;
    size_t numThreads_;
    std::atomic_bool started_;
    InetAddress local_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback threadInitCallback_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

} // namespace ev

} // namespace mudong