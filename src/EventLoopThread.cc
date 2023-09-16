#include "EventLoopThread.hpp"

#include "EventLoop.hpp"

using namespace mudong::ev;

EventLoopThread::EventLoopThread()
        : started_(false),
          loop_(nullptr),
          latch_(1)
{}

EventLoopThread::~EventLoopThread() {
    if (started_) {
        if (loop_ != nullptr) {
            loop_->quit();
        }
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!started_);
    started_ = true;

    assert(loop_ == nullptr);
    thread_ = std::thread([this](){runInThread();});
    // 强制规定了执行顺序，只有当runInThread中的latch_.count()执行后，即loop_赋值后，才会继续执行
    latch_.wait();
    assert(loop_ != nullptr);
    return loop_;
}

void EventLoopThread::runInThread() {
    EventLoop loop; // 该EventLoop对象和所属的EventLoopThread没有运行在同一个线程里，是线程局部变量
    loop_ = &loop;
    latch_.count();
    loop.loop(); // EventLoopThread对象析构时，loop才会停止
    loop_ = nullptr;
}