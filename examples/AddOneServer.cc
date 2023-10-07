#include <iostream>
#include <future>
#include <string>
#include <unordered_map>

#include <TcpServer.hpp>
#include <EventLoop.hpp>
#include <Logger.hpp>
#include <TcpConnection.hpp>
#include <ThreadPool.hpp>

using namespace mudong::ev;

using namespace std::chrono;

class AddOneServer : noncopyable {

public:
    AddOneServer(EventLoop* loop, const InetAddress& addr, size_t threadNums = 1, Nanoseconds timeout = 30s, size_t threadPoolSize = 8)
            : loop_(loop),
              server_(loop, addr),
              threadNums_(threadNums),
              timeout_(timeout),
              timer_(loop->runEvery(timeout_, [this](){onTimeout();})),
              threadPool_(threadPoolSize)
    {
        server_.setConnectionCallback(std::bind(&AddOneServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&AddOneServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
        server_.setWriteCompleteCallback(std::bind(&AddOneServer::onWriteComplete, this, std::placeholders::_1));
    }

    ~AddOneServer() {
        loop_->cancelTimer(timer_);
    }

    void start() {
        server_.setNumThread(threadNums_);
        server_.start();
    }

    void onConnection(const TcpConnectionPtr& conn) {
        INFO("connection {} is {}", conn->name(), conn->connected() ? "up" : "down");

        if (conn->connected()) {
            conn->setHighWaterMarkCallback(std::bind(&AddOneServer::onHighWaterMark, this, std::placeholders::_1, std::placeholders::_2), 1024);
        }
        else connections_.erase(conn);
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
        TRACE("connection {} recv {} byte(s)", conn->name(), buffer.readableBytes());

        int tid = static_cast<pid_t>(syscall(SYS_gettid));
        conn->send("Connection handled by tid " + std::to_string(tid) + ", ");

        std::promise<long long> res_promise;
        std::future<long long> res_future = res_promise.get_future();

        threadPool_.runTask([&](){
            long long oldNum = std::stoll(buffer.retrieveAllAsString());
            int ttid = static_cast<pid_t>(syscall(SYS_gettid));
            buffer.append("calculation handled by tid " + std::to_string(ttid) + ": ");
            res_promise.set_value(oldNum + 1);
        });

        std::string res = std::to_string(res_future.get());
        buffer.append(res + "\n");
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

    using ConnectionList = std::unordered_map<TcpConnectionPtr, Timestamp>;
    
    EventLoop* loop_;
    TcpServer server_;
    const size_t threadNums_;
    const Nanoseconds timeout_;
    Timer* timer_;
    ConnectionList connections_;
    ThreadPool threadPool_;
};

int main() {
    setLogLevel(LOG_LEVEL::LOG_LEVEL_TRACE);
    EventLoop loop;
    InetAddress local(9877);
    AddOneServer server(&loop, local, 32, 30s, 32);
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

    return 0;
}