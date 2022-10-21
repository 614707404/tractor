#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include <tractor/Socket.h>
#include <tractor/SockAddr.h>
#include <functional>
#include <tractor/Channel.h>

namespace tractor
{
    class EventLoop;
    class Acceptor
    {
    public:
        typedef std::function<void(int sockfd, const SockAddr &)> NewConnectionCallback;

        Acceptor(EventLoop *loop, SockAddr &listenAddr);
        ~Acceptor();
        void Listen();
        bool Listening() const { return listening_; }
        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            newConnectionCallback_ = cb;
        }

    private:
        void handleRand();

        EventLoop *loop_;
        Socket acceptSocket_;

        Channel acceptChannel_;

        NewConnectionCallback newConnectionCallback_;
        bool listening_;
        // int idleFd_; // idle，闲置
    };

}
#endif