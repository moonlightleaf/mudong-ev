#include "TcpClient.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"
#include "TcpConnection.hpp"

using namespace std::chrono;
using namespace mudong::ev;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& peer)
        : loop_(loop),
          connected_(false),
          peer_(peer),
          retryTimer_(nullptr),
          connector_(new Connector(loop, peer)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback)
{
    connector_->setNewConnectionCallback(std::bind(
            &TcpClient::newConnection, 
            this, 
            std::placeholders::_1, 
            std::placeholders::_2,
            std::placeholders::_3
    ));
}

TcpClient::~TcpClient() {
    if (connection_ && !connection_->disconnected()) {
        connection_->forceClose();
    }
    if (retryTimer_ != nullptr) {
        loop_->cancelTimer(retryTimer_);
    }
}

void TcpClient::setConnectionCallback(const ConnectionCallback& callback) {
    connectionCallback_ = callback;
}
void TcpClient::setMessageCallback(const MessageCallback& callback) {
    messageCallback_ = callback;
}
void TcpClient::setWriteCompleteCallback(const WriteCompleteCallback& callback) {
    writeCompleteCallback_ = callback;
}
void TcpClient::setErrorCallback(const ErrorCallback& callback) {
    connector_->setErrorCallback(callback);
}

void TcpClient::start() {
    loop_->assertInLoopThread();
    connector_->start();
    retryTimer_ = loop_->runEvery(3s, [this](){retry();});
}

void TcpClient::retry() {
    loop_->assertInLoopThread();
    if (connected_) return;

    WARN("TcpClient::retry() reconnect {}...", peer_.toIpPort());
    connector_ = std::make_unique<Connector>(loop_, peer_);
    connector_->setNewConnectionCallback(std::bind(
            &TcpClient::newConnection, 
            this, 
            std::placeholders::_1, 
            std::placeholders::_2, 
            std::placeholders::_3
    ));
    connector_->start();
}

void TcpClient::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();
    loop_->cancelTimer(retryTimer_);
    retryTimer_ = nullptr;
    connected_ = true;
    auto conn = std::make_shared<TcpConnection>(loop_, connfd, local, peer);
    connection_ = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::closeConnection, this, std::placeholders::_1));

    conn->connectEstablished();
    connectionCallback_(conn);
}

void TcpClient::closeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(connection_ != nullptr);
    connection_.reset();
    connectionCallback_(conn);
}