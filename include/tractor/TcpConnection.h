#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include <string>
#include <tractor/noncopyable.h>
#include <tractor/SockAddr.h>
#include <tractor/Channel.h>
#include <tractor/Socket.h>
#include <boost/scoped_ptr.hpp>
namespace tractor
{
    // TcpConnection 封装一个Tcp连接
    class TcpConnection : public noncopyable,
                          public std::enable_shared_from_this<TcpConnection>
    {
    public:
        typedef std::function<void(const std::shared_ptr<TcpConnection> &)> ConnectionCallback;
        typedef std::function<void(const std::shared_ptr<TcpConnection> &, const char *data, ssize_t len)> MessageCallback;
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);
        ~TcpConnection();

        EventLoop *getLoop() const { return loop_; }
        const std::string &getName() const { return name_; }
        const SockAddr &getLocalAddr() { return localAddr_; }
        const SockAddr &getPeerAddr() { return peerAddr_; }
        bool connected() const { return state_ == kConnected; }

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

        void connectEstablished();

    private:
        enum ConnectState_
        {
            kConnecting,
            kConnected,
        };

        void setState_(ConnectState_ s);
        void handleRead_();

        EventLoop *loop_;
        std::string name_;
        ConnectState_ state_;

        SockAddr localAddr_;
        SockAddr peerAddr_;

        boost::scoped_ptr<Channel> channel_;
        boost::scoped_ptr<Socket> socket_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
    };
}
#endif