#include <tractor/SockAddr.h>
#include <cstring>
using namespace tractor;
using namespace std;
SockAddr::SockAddr() {}

SockAddr::SockAddr(const char *ip, unsigned short port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = port;
    inet_aton(ip, &addr_.sin_addr);
    // TODO ret <= 0
}
SockAddr::SockAddr(unsigned short port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port); // converts the unsigned short integer hostshort from host byte order to network byte order.
}

SockAddr::~SockAddr() {}

struct sockaddr_in SockAddr::getAddr() const
{
    return addr_;
}

struct sockaddr_in *SockAddr::getAddrPtr()
{
    return &addr_;
}

string SockAddr::toString() const
{
    char buf[32];
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr_.sin_addr, host, sizeof host);
    uint16_t port = ntohs(addr_.sin_port); // 一定要做一次转换
    // uint16_t port = addr_.sin_port;
    snprintf(buf, 32, "%s:%u", host, port);
    return buf;
}