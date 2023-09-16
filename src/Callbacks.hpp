#pragma once

#include <memory>
#include <functional>

namespace mudong {

namespace ev {

class Buffer;
class TcpConnection;
class InetAddress;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer&)>;
using ErrorCallback = std::function<void()>;
using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& local, const InetAddress& peer)>;
using Task = std::function<void()>;
using ThreadInitCallback = std::function<void(size_t)>;
using TimerCallback = std::function<void()>;

void defaultThreadInitCallback(size_t);
void defaultConnectionCallback(const TcpConnectionPtr&);
void defaultMessageCallback(const TcpConnectionPtr&, Buffer&);

} // namespace ev

} // namespace mudong