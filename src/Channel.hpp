#pragma once

#include "noncopyable.hpp"

#include <functional>
#include <memory>


namespace mudong {

namespace ev {

class EventLoop;

class Channel : noncopyable {

private:
    using ReadCallback  = std::function<void()>;
    using WriteCallback = std::function<void()>;
    using CloseCallback = std::function<void()>;
    using ErrorCallback = std::function<void()>;

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

public:
    void setReadCallback(const ReadCallback& callback);
    void setWriteCallback(const WriteCallback& callback);
    void setCloseCallback(const CloseCallback& callback);
    void setErrorCallback(const ErrorCallback& callback);

    // TODO
    void handleEvents();
    int fd() const;
    bool isNoneEvents() const;
    unsigned events() const;
    void setRevents(unsigned);
    void tie(const std::shared_ptr<void>& obj);

    void enableRead();
    void enableWrite();
    void disableRead();
    void disableWrite();
    void disableAll();

    bool isReading() const;
    bool isWriting() const;

public:
    bool pooling;

private:
    EventLoop* loop_;
    int fd_;
    std::weak_ptr<void> tie_;
    bool tied_;
    unsigned events_;
    unsigned revents_;
    bool handlingEvents_;

    ReadCallback readCallback_;
    WriteCallback writeCallback_;
    CloseCallback closeCallback_;
    ErrorCallback errorCallback_;

private:
    // TODO
    void update();
    void remove();

    void handleEventWithGuard();

};

} // namespace ev

} // namespace mudong