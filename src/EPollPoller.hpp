#pragma once

#include <vector>
#include <sys/epoll.h>

#include "noncopyable.hpp"

namespace mudong {

namespace ev {

class EventLoop;
class Channel;

class EPollPoller: noncopyable {

public:
    using ChannelList = std::vector<Channel*>;

    explicit EPollPoller(EventLoop* loop);
    ~EPollPoller();

    void poll(ChannelList& activeChannels);
    void updateChannel(Channel* channel);

private:
    void updateChannel(int op, Channel* channel);
    EventLoop* loop_;
    std::vector<epoll_event> events_;
    int epollfd_;

};

} // namespace ev

} // namespace mudong