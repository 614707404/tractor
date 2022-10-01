#ifndef SOCKET_H
#define SOCKET_H
#include <tractor/SockAddr.h>
#include <unistd.h>
namespace tractor
{
    class Socket
    {
    public:
        Socket();
        ~Socket();
        void Bind(tractor::SockAddr &ServerAddr);
        void Listen();
        int Accept(tractor::SockAddr &ClientAddr);

        int getFd();
        // void setTcpNoDelay(bool enable);
        // int shutdownWrite();

    private:
        int socketFd_;
    };
} // namespace tractor

#endif