#pragma once

#include "noncopyable.hpp"
#include "Channel.hpp"
#include "InetAddress.hpp"
#include "Callbacks.hpp"

namespace mudong {

namespace ev {

class EventLoop;
class InetAddress;

class Connector: noncopyable {

public:
    Connector(EventLoop* loop, const InetAddress& peer);
    ~Connector();

    void start();

    void setNewConnectionCallback(const NewConnectionCallback& callback);
    void setErrorCallback(const ErrorCallback& callback);

private:
    void handleWrite();

    EventLoop* loop_;
    const InetAddress peer_;
    const int sockfd_;
    bool connected_;
    bool started_;
    Channel channel_;
    NewConnectionCallback newConnectionCallback_;
    ErrorCallback errorCallback_;
};

} // namespace ev

} // namespace mudong