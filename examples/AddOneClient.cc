#include <iostream>
#include <thread>
#include "TcpClient.hpp"
#include "Logger.hpp"
#include "TcpConnection.hpp"
#include "EventLoop.hpp"

using namespace mudong::ev;

class UserInput : noncopyable {

public:
    UserInput(const TcpConnectionPtr& conn) : conn_(conn) {
        conn_->setMessageCallback(std::bind(&UserInput::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
        std::cout << buffer.retrieveAllAsString() << std::endl;
    }

    void run() {
        std::string line;
        while (std::getline(std::cin, line)) {
            conn_->send(line);
        }
        conn_->shutdown();
    }

private:
    TcpConnectionPtr conn_;
};

class AddOneBench : noncopyable {

public:
    AddOneBench(EventLoop* loop, const InetAddress& addr) : loop_(loop), client_(loop, addr) {
        client_.setConnectionCallback(std::bind(&AddOneBench::onConnection, this, std::placeholders::_1));
    }

    void start() {
        client_.start();
    }

    void onConnection(const TcpConnectionPtr& conn) {
        INFO("connection {} is [{}]", conn->name(), conn->connected() ? "up" : "down");

        if (conn->connected()) {
            auto th = std::thread([conn](){
                UserInput user(conn);
                user.run();
            });
            th.detach();
        }
        else {
            loop_->quit();
        }
    }
private:
    EventLoop* loop_;
    TcpClient client_;
};

int main() {
    setLogLevel(LOG_LEVEL::LOG_LEVEL_WARN);
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9877);
    AddOneBench client(&loop, addr);
    client.start();
    loop.loop();
}