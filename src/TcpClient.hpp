#pragma once

#include "Callbacks.hpp"
#include "Connector.hpp"
#include "Timer.hpp"

namespace mudong {

namespace ev {

class TcpClient : noncopyable {

public:
    TcpClient(EventLoop* loop, const InetAddress& peer);
    ~TcpClient();

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);
    void setErrorCallback(const ErrorCallback&);

    void start();

private:
    void retry();
    void newConnection(int connfd, const InetAddress& local, const InetAddress& peer);
    void closeConnection(const TcpConnectionPtr& conn);

    using ConnectorPtr = std::unique_ptr<Connector>;

    EventLoop* loop_;
    bool connected_;
    const InetAddress peer_;
    Timer* retryTimer_;
    ConnectorPtr connector_;
    TcpConnectionPtr connection_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

} // namespace ec

} // namespace mudong