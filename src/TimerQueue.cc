#include <sys/timerfd.h>
#include <chrono>

#include "TimerQueue.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"

using namespace mudong::ev;

namespace {

int timerfdCreate() {
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1) {
        SYSFATAL("timerfd_create");
    }
    return fd;
}

void timerfdRead(int fd) {
    uint64_t val;
    ssize_t n = read(fd, &val, sizeof(val));
    if (n != sizeof(val)) {
        ERROR("timerfdRead get length{}, not {}", n, sizeof(val));
    }
}

// 返回从当前时间点到指定时间戳之间的时间间隔
timespec durationFromNow(Timestamp when) {
    timespec ret;
    Nanoseconds ns = when - clock::now();

    using namespace std::chrono;
    if (ns < 1ms) ns = 1ms;

    // 分别设置秒数和纳秒数
    ret.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
    ret.tv_nsec = ns.count() % std::nano::den;
    return ret;
}

/*
struct timespec {
    time_t tv_sec;  // 秒数
    long tv_nsec;   // 纳秒数
};

struct itimerspec {
    struct timespec it_interval;  // 定时器重复间隔时间
    struct timespec it_value;     // 定时器初次到期时间，以倒计时的方式存储
};
*/

void timerfdSet(int fd, Timestamp when) {
    itimerspec oldtime, newtime;
    memset(&oldtime, 0, sizeof(itimerspec));
    memset(&newtime, 0, sizeof(itimerspec));
    newtime.it_value = durationFromNow(when);

    int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
    if (ret == -1) {
        SYSERR("timerfd_settime");
    }
}

} // anonymous namespace

TimerQueue::TimerQueue(EventLoop* loop)
        : loop_(loop),
          timerfd_(timerfdCreate()),
          timerChannel_(loop, timerfd_)
{
    loop_->assertInLoopThread();
    timerChannel_.setReadCallback([this](){this->handleRead();}); // 定时器触发时，timerFd_会有可读事件，交由handleRead来处理
    timerChannel_.enableRead();
}

TimerQueue::~TimerQueue() {
    for (auto& p : timers_) {
        delete p.second;
    }
    close(timerfd_);
}

Timer* TimerQueue::addTimer(TimerCallback callback, Timestamp when, Nanoseconds interval) {
    Timer* timer = new Timer(std::move(callback), when, interval);
    loop_->runInLoop(
        [this, when, timer]() {
            auto checkPair = timers_.insert({when, timer});
            assert(checkPair.second);
            // 新插入的就是时间最邻近的，那么就设置定时器触发时刻为最近的时间点
            if (timers_.begin() == checkPair.first) {
                timerfdSet(timerfd_, when);
            }
        }
    );
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer) {
    loop_->runInLoop(
        [this, timer]() {
            timer->cancel();
            timers_.erase({timer->when(), timer});
            delete timer;
        }
    );
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    timerfdRead(timerfd_); // 将可读的内容读取一下从而清空缓冲区

    Timestamp now(clock::now());
    for (auto& e : getExpired(now)) {
        Timer* timer = e.second;
        assert(timer->expired(now)); // now >= when

        if (!timer->canceled()) {
            timer->run();
        }
        if (!timer->canceled() && timer->repeat()) {
            timer->restart();
            e.first = timer->when(); // 如果需要重复，那就按interval设置新的时间戳，并加入定时器管理集合
            timers_.insert(e);
        }
        else delete timer; //否则说明已经被取消了，直接丢弃
    }

    if (!timers_.empty()) {
        timerfdSet(timerfd_, timers_.begin()->first); // 如果还有未到时间的定时器，那就把最近的时间点作为触发时间点（向内核中注册的定时器其实只有一个，定时器队列由网络库维护）
    }
}

// 将已过期的定时器从set中移除，并返回已过期的Entry数组
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    using namespace std::chrono;
    Entry endEntry(now + 1ns, nullptr);
    std::vector<Entry> ret;

    auto end = timers_.lower_bound(endEntry);
    ret.assign(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);
    return ret;
}