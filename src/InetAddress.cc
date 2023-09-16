#include <arpa/inet.h>
#include <strings.h>

#include "Logger.hpp"
#include "InetAddress.hpp"

using namespace mudong::ev;

InetAddress::InetAddress(uint16_t port, bool loopback) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopback ? INADDR_LOOPBACK:INADDR_ANY; //是否设为回环地址127.0.0.1
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr);
    if (ret != 1) {
        SYSFATAL("InetAddress::inet_pton");
    }
    addr_.sin_port = htons(port);
}

void InetAddress::setAddress(const sockaddr_in& addr) {
    addr_ = addr;
}

const sockaddr* InetAddress::getSockaddr() const {
    return reinterpret_cast<const sockaddr*>(&addr_);
}

socklen_t InetAddress::getSocklen() const {
    return sizeof(addr_);
}

std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    const char* ret = inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    if (ret == nullptr) {
        buf[0] = '\0';
        SYSERR("InetAddress::inet_ntop");
    }
    return std::string(buf);
}

uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const {
    std::string ret = toIp();
    ret.push_back(':');
    ret += std::to_string(toPort());
    return ret;
}