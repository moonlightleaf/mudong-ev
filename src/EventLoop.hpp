#pragma once

#include <atomic>

#include "Timer.hpp"
#include "TimerQueue.hpp"
#include "EPollPoller.hpp"

namespace mudong {

namespace ev {

class EventLoop: noncopyable {

public:
    EventLoop();
    ~EventLoop();
    
    // 开启循环
    void loop();
    // 退出循环
    void quit();

    // 在当前loop中执行
    void runInLoop(const Task& task);
    void runInLoop(Task&& task);
    // 把任务放入队列中，唤醒loop所在的线程执行task
    void queueInLoop(const Task& task);
    void queueInLoop(Task&& task);

    Timer* runAt(Timestamp when, TimerCallback callback);
    Timer* runAfter(Nanoseconds interval, TimerCallback callback);
    Timer* runEvery(Nanoseconds interval, TimerCallback callback);
    void cancelTimer(Timer* timer);

    // 通过wakeupfd_/wakeupChannel_唤醒loop所在的线程
    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    // 判断EventLoop对象是否在自己的线程里。可能在别的线程中被调用
    void assertInLoopThread();
    void assertNotInLoopThread();
    bool isInLoopThread();

private:
    // 执行上层添加的任务
    void doPendingTasks();
    // 与wakeupfd_/wakeupChannel_绑定的回调，构造EventLoop时绑定
    void handleRead();
    // EventLoop对象创建时所在的线程，用以判断当前EventLoop对象是否在自身所属的线程中
    const pid_t tid_;
    std::atomic_bool quit_;
    std::atomic_bool doingPendingTasks_;
    EPollPoller poller_;
    EPollPoller::ChannelList activeChannels_;
    const int wakeupfd_;
    Channel wakeupChannel_;
    std::mutex mutex_;
    std::vector<Task> pendingTasks_;
    TimerQueue timerQueue_;
};

} // namespace ev

} // namespace mudong