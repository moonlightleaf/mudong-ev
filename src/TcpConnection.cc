#include "TcpConnection.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"

using namespace mudong::ev;

namespace {

enum ConnectState {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected
};

} // anonymous namespace

namespace mudong {

namespace ev {

void defaultThreadInitCallback(size_t index) {
    TRACE("EventLoop thread {} started", index);
}
void defaultConnectionCallback(const TcpConnectionPtr& conn) {
    INFO("Connection {} -> {} {}", conn->peer().toIpPort(), conn->local().toIpPort(), conn->connected() ? "up" : "down");
}
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer& buffer) {
    TRACE("Connection {} -> {} recv {} bytes", conn->peer().toIpPort(), conn->local().toIpPort(), buffer.readableBytes());
    buffer.retrieveAll();
}

} // namespace ev

} // namespace mudong

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, const InetAddress& local, const InetAddress& peer)
        : loop_(loop),
          sockfd_(sockfd),
          channel_(loop, sockfd_),
          state_(kConnecting),
          local_(local),
          peer_(peer),
          highWaterMark_(0)
{
    channel_.setReadCallback([this](){handleRead();});
    channel_.setWriteCallback([this](){handleWrite();});
    channel_.setCloseCallback([this](){handleClose();});
    channel_.setErrorCallback([this](){handleError();});

    TRACE("TcpConnection() {} fd={}", name(), sockfd_);
}

TcpConnection::~TcpConnection() {
    assert(state_ == kDisconnected);
    close(sockfd_);

    TRACE("~TcpConnection() {} fd={}", name(), sockfd_);
}

void TcpConnection::setMessageCallback(const MessageCallback& callback) {
    messageCallback_ = callback;
}
void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& callback) {
    writeCompleteCallback_ = callback;
}
void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& callback, size_t mark) {
    highWaterMark_ = mark;
    highWaterMarkCallback_ = callback;
}
void TcpConnection::setCloseCallback(const CloseCallback& callback) {
    closeCallback_ = callback;
}

void TcpConnection::connectEstablished() {
    assert(state_ == kConnecting);
    state_ = kConnected;
    channel_.tie(shared_from_this()); // 将socketfd_的Channel和TcpConnection绑定
    channel_.enableRead(); // 打开socket的读
}
bool TcpConnection::connected() const {
    return state_ == kConnected;
}
bool TcpConnection::disconnected() const {
    return state_ == kDisconnected;
}

const InetAddress& TcpConnection::local() const {
    return local_;
}
const InetAddress& TcpConnection::peer() const {
    return peer_;
}
std::string TcpConnection::name() const {
    return peer_.toIpPort() + " -> " + local_.toIpPort();
}

void TcpConnection::setContext(const std::any& context) {
    context_ = context;
}
const std::any& TcpConnection::getContext() const {
    return context_;
}
std::any& TcpConnection::getContext() {
    return context_;
}

void TcpConnection::send(std::string_view data) {
    send(data.data(), data.length());
}
void TcpConnection::send(const char* data, size_t len) {
    if (state_ != kConnected) {
        WARN("TcpConnection::send() not connected, give up send");
        return;
    }
    // 在自己所属的EventLoop中
    if (loop_->isInLoopThread()) {
        sendInLoop(data, len);
    }
    // 多Reactor，没有在自己所属的EventLoop中
    else {
        loop_->queueInLoop(
                [ptr = shared_from_this(), str = std::string(data, data+len)]()
                { ptr->sendInLoop(str);});
    }
}
void TcpConnection::send(Buffer& buffer) {
    if (state_ != kConnected) {
        WARN("TcpConnection::send() not connected, give up send");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(buffer.peek(), buffer.readableBytes());
        buffer.retrieveAll();
    }
    else {
        loop_->queueInLoop([ptr = shared_from_this(), str = buffer.retrieveAllAsString()](){ptr->sendInLoop(str);});
    }
}

void TcpConnection::shutdown() {
    assert(state_ != kDisconnected);
    if (stateAtomicGetAndSet(kDisconnecting) == kConnected) {
        if (loop_->isInLoopThread()) {
            shutdownInLoop();
        }
        else {
            // 成员函数第一个隐藏形参为this指针
            loop_->queueInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
        }
    }
}
void TcpConnection::forceClose() {
    if (state_ != kDisconnected) {
        if (stateAtomicGetAndSet(kDisconnecting) != kDisconnected) {
            loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
        }
    }
}

void TcpConnection::stopRead() {
    loop_->runInLoop([this]() {
        if (channel_.isReading()) {
            channel_.disableRead();
        }
    });
}
void TcpConnection::startRead() {
    loop_->runInLoop([this]() {
        if (!channel_.isReading()) {
            channel_.enableRead();
        }
    });
}
bool TcpConnection::isReading() {
    return channel_.isReading();
}

const Buffer& TcpConnection::inputBuffer() const {
    return inputBuffer_;
}
const Buffer& TcpConnection::outputBuffer() const {
    return outputBuffer_;
}

void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    assert(state_ != kDisconnected);
    int savedErrno;
    ssize_t n = inputBuffer_.readFd(sockfd_, &savedErrno);
    if (n == -1) {
        errno = savedErrno;
        SYSERR("TcpConnection::read()");
        handleError();
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        messageCallback_(shared_from_this(), inputBuffer_);
    }
}
void TcpConnection::handleWrite() {
    if (state_ == kDisconnected) {
        WARN("TcpConnection::handleWrite() disconnected, give up writing {} bytes", outputBuffer_.readableBytes());
        return;
    }
    assert(outputBuffer_.readableBytes() > 0);
    assert(channel_.isWriting());
    ssize_t n = ::write(sockfd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
    if (n == -1) {
        SYSERR("TcpConnection::write()");
    }
    else {
        outputBuffer_.retrieve(static_cast<size_t>(n));
        if (outputBuffer_.readableBytes() == 0) {
            channel_.disableWrite();
            if (state_ == kDisconnecting)
                shutdownInLoop();
            if (writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
    }
}
void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    state_ = kDisconnected;
    loop_->removeChannel(&channel_);
    closeCallback_(shared_from_this());
}
void TcpConnection::handleError() {
    int err;
    socklen_t len = sizeof(err);
    int ret = getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len);
    if (ret != -1)
        errno = err;
    SYSERR("TcpConnection::handleError()");
}

void TcpConnection::sendInLoop(const char *data, size_t len) {
    loop_->assertInLoopThread();
    if (state_ == kDisconnected) {
        WARN("TcpConnection::sendInLoop() disconnected, give up send");
        return;
    }
    ssize_t n = 0;
    size_t remain = len;
    bool faultError = false;
    /**
     * 如果已经在监听EPOLLOUT事件了，说明sockfd_内核缓冲区已经是已满状态，因此就不会尝试执行write
     * 的操作，而是直接执行下面的逻辑，将待发送数据追加到outputBuffer_ 
    **/
    if (!channel_.isWriting()) {
        assert(outputBuffer_.readableBytes() == 0);
        n = ::write(sockfd_, data, len);
        if (n == -1) {
            if (errno != EAGAIN) {
                SYSERR("TcpConnection::write()");
                if (errno == EPIPE || errno == ECONNRESET)
                    faultError = true;
            }
            n = 0;
        }
        else {
            remain -= static_cast<size_t>(n);
            if (remain == 0 && writeCompleteCallback_) {
                // 正常写完了，执行写完成回调
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
    }
    /**
     * 如果没有发生write错误，并且还有数据没有写完，说明sockfd_的内核缓冲区已满，只写了一部分进去，
     * 此刻将会触发高水位回调，并且注册sockfd_(channel_)的EPOLLOUT事件，当channel_中有数据出去后，
     * 内核缓冲区将有空余空间，此时将之前没写完，暂存在outputBuffer_中的数据继续向sockfd_的内核
     * 缓冲区写入
    **/
    if (!faultError && remain > 0) {
        if (highWaterMarkCallback_) {
            size_t oldLen = outputBuffer_.readableBytes();
            size_t newLen = oldLen + remain;
            if (oldLen < highWaterMark_ && newLen >= highWaterMark_)
                loop_->queueInLoop(std::bind(
                        highWaterMarkCallback_, shared_from_this(), newLen));
        }
        outputBuffer_.append(data + n, remain);
        channel_.enableWrite();
    }
}
void TcpConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.data(), message.length());
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (state_ != kDisconnected && !channel_.isWriting()) {
        if (::shutdown(sockfd_, SHUT_WR) == -1) {
            SYSERR("TcpConnection::shutdown()");
        }
    }
}
void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ != kDisconnected) {
        handleClose();
    }
}

int TcpConnection::stateAtomicGetAndSet(int newState) {
    return __atomic_exchange_n(&state_, newState, __ATOMIC_SEQ_CST); // 顺序一致性内存序
}