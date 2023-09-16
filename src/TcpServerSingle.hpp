#pragma once

#include <unordered_set>

#include "Callbacks.hpp"
#include "Acceptor.hpp"

namespace mudong {

namespace ev {

class EventLoop;

class TcpServerSingle : noncopyable {

public:
    TcpServerSingle(EventLoop* loop, const InetAddress& local);

    void setConnectionCallback(const ConnectionCallback& callback);
    void setMessageCallback(const MessageCallback &callback);
    void setWriteCompleteCallback(const WriteCompleteCallback &callback);
    
    void start();

private:
    using ConnectionSet = std::unordered_set<TcpConnectionPtr>;

    void newConnection(int connfd, const InetAddress &local, const InetAddress &peer);

    void closeConnection(const TcpConnectionPtr &conn);

    EventLoop* loop_;
    Acceptor acceptor_;
    ConnectionSet connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

} // namespace ev

} // namespace mudong