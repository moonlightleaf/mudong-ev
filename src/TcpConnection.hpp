#pragma once

#include <any>

#include "noncopyable.hpp"
#include "Callbacks.hpp"
#include "Channel.hpp"
#include "InetAddress.hpp"
#include "Buffer.hpp"

namespace mudong {

namespace ev {

class EventLoop;

class TcpConnection: noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, int sockfd, const InetAddress& local, const InetAddress& peer);
    ~TcpConnection();

    void setMessageCallback(const MessageCallback& callback);
    void setWriteCompleteCallback(const WriteCompleteCallback& callback);
    void setHighWaterMarkCallback(const HighWaterMarkCallback& callback, size_t mark);
    void setCloseCallback(const CloseCallback& callback);

    void connectEstablished();
    bool connected() const;
    bool disconnected() const;

    const InetAddress& local() const;
    const InetAddress& peer() const;
    std::string name() const;

    void setContext(const std::any& context);
    const std::any& getContext() const;
    std::any& getContext();

    void send(std::string_view data);
    void send(const char* data, size_t len);
    void send(Buffer& buffer);

    void shutdown(); // 半关闭，关闭服务端写，保留读
    void forceClose();

    void stopRead();
    void startRead();
    bool isReading();

    const Buffer& inputBuffer() const;
    const Buffer& outputBuffer() const;

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const char* data, size_t len);
    void sendInLoop(const std::string& message);
    void shutdownInLoop();
    void forceCloseInLoop();

    int stateAtomicGetAndSet(int newState);

    EventLoop* loop_;
    const int sockfd_;
    Channel channel_;
    int state_;
    InetAddress local_;
    InetAddress peer_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    size_t highWaterMark_;
    std::any context_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
};

} // namespace ev

} // namespace mudong