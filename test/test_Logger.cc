#include <Logger.hpp>

#include <iostream>
#include <chrono>
#include <thread>


int main() {
    mudong::ev::setLogLevel(mudong::ev::LOG_LEVEL::LOG_LEVEL_DEBUG);

    INFO("server quit after {} second...", 10);
    DEBUG("DEBUG log");

    mudong::ev::setLogFile("./test_Logger.log");
    ERROR("something wrong test {} {} {}", 1, 2, 3);

    mudong::ev::setLogFile("stdout");
    SYSERR("someting sys wrong test {} {} {}", 1, 2, 3);


    std::cout << "ready to abort" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << 3 << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << 2 << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << 1 << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    SYSFATAL("ready to abort{} {} {} {}", ":", 3, 2, 1);


    return 0;
}
