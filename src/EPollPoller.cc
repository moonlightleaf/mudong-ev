#include "EPollPoller.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"

using namespace mudong::ev;

EPollPoller::EPollPoller(EventLoop* loop)
        : loop_(loop),
          events_(128),
          epollfd_(epoll_create1(EPOLL_CLOEXEC))
{
    if (epollfd_ == -1) {
        SYSFATAL("Epoller::epoll_create1");        
    }
}
EPollPoller::~EPollPoller() {
    close(epollfd_);
}

void EPollPoller::poll(ChannelList& activeChannels) {
    loop_->assertInLoopThread();
    int maxEvents = static_cast<int>(events_.size());
    // 得到触发的event个数，并将epoll_event写入events_缓冲区，最多一次获取128个(初始参数，可调)
    int nEvents = epoll_wait(epollfd_, events_.data(), maxEvents, -1);
    if (nEvents == -1) {
        if (errno != EINTR) { // signal: interrupted sys call
            SYSERR("EPollPoller::epoll_wait");
        }
    }
    else if (nEvents > 0) {
        // 得到触发的event个数，并依次访问操作
        for (int i = 0; i < nEvents; ++i) {
            //epoll_event中的epoll_data_t中存对应Channel的指针
            auto channelPtr = static_cast<Channel*>(events_[i].data.ptr); 
            channelPtr->setRevents(events_[i].events); // 事件类型
            activeChannels.push_back(channelPtr); //把该Channel放入活跃队列中
        }
        if (nEvents == maxEvents) {
            events_.resize(2 * events_.size()); // 扩容
        }
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    loop_->assertInLoopThread();
    int op = 0;
    // 更新Channel的几种情况
    if (!channel->pooling) { // 如果当前Channel没有被管理，那么这里的更新操作一定是要添加到epoll管理
        assert(!channel->isNoneEvents()); // 既然是新的待添加管理的对象，那么一定是event，设置过想要监听的操作类型
        op = EPOLL_CTL_ADD;
        channel->pooling = true;
    }
    else if (!channel->isNoneEvents()) { // 如果已经被管理了，并且传入的仍然是event，说明是想修改已注册的epfd上的操作或属性
        op = EPOLL_CTL_MOD;
    }
    else { // 如果已经被管理，并且传入的不是一个event，表示想要将该注册过的Channel给删除
        op = EPOLL_CTL_DEL;
        channel->pooling = false;
    }
    updateChannel(op, channel);
}

void EPollPoller::updateChannel(int op, Channel* channel) {
    epoll_event epEv;
    epEv.events = channel->events();
    // 在注册事件的时候，附带上了Channel指针，因此epoll_wait得到的event中包含了
    // 事件和对应的Channel指针，因此，不需要单独存储fd->Channel*的映射
    epEv.data.ptr = channel;
    if (epoll_ctl(epollfd_, op, channel->fd(), &epEv) == -1) {
        SYSERR("EPollPoller::epoll_ctl");
    }
}