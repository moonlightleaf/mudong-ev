#pragma once

#include <memory>
#include <set>

#include "Timer.hpp"
#include "Channel.hpp"
#include "Timestamp.hpp"
#include "noncopyable.hpp"

namespace mudong {

namespace ev {

class TimerQueue: noncopyable {

public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimerCallback callback, Timestamp when, Nanoseconds interval);
    void cancelTimer(Timer* timer);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerList timers_;
};

} // namespace ev

} // namespace mudong