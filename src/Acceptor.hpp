#pragma once

#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include "Channel.hpp"
#include "Callbacks.hpp"

namespace mudong {

namespace ev {

class EventLoop;

class Acceptor: noncopyable {

public:
    Acceptor(EventLoop*, const InetAddress&);
    ~Acceptor();

    bool listening() const;

    void listen();

    void setNewConnectionCallback(const NewConnectionCallback& callback);

private:
    void handleRead();
    
    bool listening_;
    EventLoop* loop_; // 指向的是主Reactor的EventLoop对象
    const int acceptfd_;
    Channel acceptChannel_;
    InetAddress local_;
    NewConnectionCallback newConnectionCallback_;
};

} // namespace ev

} // namespace mudong