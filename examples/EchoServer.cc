#include <unordered_map>

#include <TcpServer.hpp>
#include <EventLoop.hpp>
#include <Logger.hpp>
#include <TcpConnection.hpp>

using namespace std::chrono;

using namespace mudong::ev;

class EchoServer : noncopyable {

public:
    EchoServer(EventLoop* loop, const InetAddress& addr, size_t threadNums = 1, Nanoseconds timeout = 10s)
            : loop_(loop),
              server_(loop, addr),
              threadNums_(threadNums),
              timeout_(timeout),
              timer_(loop_->runEvery(timeout_, [this](){onTimeout();}))
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
        server_.setWriteCompleteCallback(std::bind(&EchoServer::onWriteComplete, this, std::placeholders::_1));
    }

    ~EchoServer() {
        loop_->cancelTimer(timer_);
    }

    void start()
    {
        server_.setNumThread(threadNums_);
        server_.start();
    }

    void onConnection (const TcpConnectionPtr& conn) {
        INFO("connection {} is {}", conn->name(), conn->connected() ? "up" : "down");

        if (conn->connected()) {
            conn->setHighWaterMarkCallback(std::bind(&EchoServer::onHighWaterMark, this, std::placeholders::_1, std::placeholders::_2), 1024);
            expireAfter(conn, timeout_);
        }
        else connections_.erase(conn);
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
        TRACE("connection {} recv {} byte(s)", conn->name(), buffer.readableBytes());

        // 发送并清空buffer
        conn->send(buffer);
        expireAfter(conn, timeout_);
    }

    void onWriteComplete(const TcpConnectionPtr& conn) {
        if (!conn->isReading()) {
            INFO("write complete, start read");
            conn->startRead();
            expireAfter(conn, timeout_);
        }
    }

    void onHighWaterMark(const TcpConnectionPtr& conn, size_t mark) {
        INFO("high water mark {} byte(s), stop read", mark);
        conn->stopRead();
        expireAfter(conn, 2 * timeout_);
    }

private:
    void onTimeout() {
        for (auto iter = connections_.begin(); iter != connections_.end(); ) {
            if (iter->second <= clock::now()) {
                INFO("connection timeout force close");
                iter->first->forceClose();
                iter = connections_.erase(iter);
            }
            else iter++;
        }
    }

    void expireAfter(const TcpConnectionPtr& conn, Nanoseconds interval) {
        connections_[conn] = clock::nowAfter(interval);
    }

    EventLoop* loop_;
    TcpServer server_;
    const size_t threadNums_;
    const Nanoseconds timeout_;
    Timer* timer_;
    using ConnectionList = std::unordered_map<TcpConnectionPtr, Timestamp>;
    ConnectionList connections_;

};

int main() {
    setLogLevel(LOG_LEVEL::LOG_LEVEL_TRACE);
    EventLoop loop;
    InetAddress local(9877);
    EchoServer server(&loop, local, 3, 20s);
    server.start();

    loop.runAfter(60s, [&]() {
        int countdown = 5;
        INFO("server quit after {} second(s)...", countdown);
        loop.runEvery(1s, [&, countdown]() mutable {
            INFO("server quit after {} second(s)...", --countdown);
            if (countdown == 0) loop.quit();
        });
    });

    loop.loop();
}