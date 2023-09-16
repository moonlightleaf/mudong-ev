#include "Logger.hpp"
#include "TcpConnection.hpp"
#include "TcpServerSingle.hpp"
#include "Buffer.hpp"
#include "EventLoop.hpp"

using namespace mudong::ev;

TcpServerSingle::TcpServerSingle(EventLoop* loop, const InetAddress& local)
        : loop_(loop),
          acceptor_(loop, local)
{
    acceptor_.setNewConnectionCallback(std::bind(&TcpServerSingle::newConnection, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void TcpServerSingle::setConnectionCallback(const ConnectionCallback& callback) {
    connectionCallback_ = callback;
}
void TcpServerSingle::setMessageCallback(const MessageCallback& callback) {
    messageCallback_ = callback;
}
void TcpServerSingle::setWriteCompleteCallback(const WriteCompleteCallback& callback) {
    writeCompleteCallback_ = callback;
}

void TcpServerSingle::start() {
    acceptor_.listen();
}

// 这里的逻辑将会传递给acceptor_.setNewConnectionCallback，当acceptfd_有可读事件触发，即有新连接请求到来时，就执行该逻辑
void TcpServerSingle::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();
    auto conn = std::make_shared<TcpConnection>(loop_, connfd, local, peer);
    connections_.insert(conn);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServerSingle::closeConnection, this, std::placeholders::_1));
    
    // 将connfd的channel tie上TcpConnection对象
    conn->connectEstablished();

    connectionCallback_(conn);
}

void TcpServerSingle::closeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    size_t ret = connections_.erase(conn);
    if (ret != 1) {
        FATAL("TcpServerSingle::closeConnection connection set erase fatal, ret = {}", ret);
    }
    connectionCallback_(conn);
}