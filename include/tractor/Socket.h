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
        Socket(int socketFd) : socketFd_(socketFd) {}
        ~Socket();
        void Bind(SockAddr &ServerAddr);
        void Listen();
        int Accept(SockAddr &ClientAddr);
        int getFd();

        // void setTcpNoDelay(bool enable);
        // int shutdownWrite();

    private:
        int socketFd_;
    };

} // namespace tractor

#endif