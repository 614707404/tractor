#include <tractor/Socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <iostream>
#include "spdlog/spdlog.h"
using namespace tractor;
using namespace std;
// Socket::Socket() : socketFd_(socket(PF_INET, SOCK_STREAM, 0)) {}
Socket::Socket() : socketFd_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) {}

Socket::~Socket()
{
    int ret = ::close(socketFd_);
    if (ret < 0)
        std::cout << "close error" << std::endl;
}

void Socket::Bind(tractor::SockAddr &ServerAddr)
{
    int ret = ::bind(socketFd_,
                     reinterpret_cast<struct sockaddr *>(ServerAddr.getAddrPtr()),
                     sizeof ServerAddr);
    if (ret < 0)
        spdlog::error("bind error");
}
void Socket::Listen()
{
    int ret = ::listen(socketFd_, SOMAXCONN);
    if (ret < 0)
        std::cout << "listen error" << std::endl;
}
int Socket::Accept(tractor::SockAddr &ClientAddr)
{
    socklen_t ClientLen = sizeof ClientAddr;
    int ret = ::accept(socketFd_,
                       reinterpret_cast<struct sockaddr *>(ClientAddr.getAddrPtr()),
                       &ClientLen);
    if (ret < 0)
        std::cout << "accept error" << std::endl;
    return ret;
}

int Socket::getFd()
{
    return socketFd_;
}
void Socket::shutdownWrite()
{
    if (::shutdown(socketFd_, SHUT_WR) < 0)
    {
        std::cout << "sockets::shutdownWrite" << std::endl;
    }
}
void Socket::setTcpNoDelay(bool enable)
{
    int optval = enable ? 1 : 0;
    ::setsockopt(socketFd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, sizeof optval);
}