#pragma once

#include <netinet/in.h>
#include <string>

namespace mudong {

namespace ev {

class InetAddress {

public:
    explicit InetAddress(uint16_t port = 0, bool loopback = false);
    InetAddress(const std::string& ip, uint16_t port);

    void setAddress(const sockaddr_in& addr);
    const sockaddr* getSockaddr() const;
    socklen_t getSocklen() const;

    std::string toIp() const;
    uint16_t toPort() const;
    std::string toIpPort() const;

private:
    sockaddr_in addr_;
};

} // namespace ev

} // namespace mudong