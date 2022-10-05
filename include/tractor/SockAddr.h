#ifndef SOCKADDR_H
#define SOCKADDR_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// 对 sockaddr_in 结构体的简单封装
/*****
#include <netinet/in.h>

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*****/

namespace tractor
{
    class SockAddr
    {
    public:
        SockAddr();
        SockAddr(const char *ip, unsigned short port);
        SockAddr(unsigned short port);
        SockAddr(sockaddr_in &addr) : addr_(addr) {}
        ~SockAddr();

        sa_family_t family() const { return addr_.sin_family; }
        struct sockaddr_in getAddr() const;
        struct sockaddr_in *getAddrPtr();

        std::string toString() const;

    private:
        struct sockaddr_in addr_;
    };

}

#endif