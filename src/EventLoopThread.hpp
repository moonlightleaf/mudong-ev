#pragma once

#include <thread>

#include "CountDownLatch.hpp"

namespace mudong {

namespace ev {

class EventLoop;

class EventLoopThread: noncopyable {

public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void runInThread();

    bool started_;
    EventLoop* loop_;
    std::thread thread_;
    CountDownLatch latch_;
};

} // namespace ev

} // namespace mudong