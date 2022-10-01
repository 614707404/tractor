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
}