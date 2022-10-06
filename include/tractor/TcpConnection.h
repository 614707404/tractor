#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <tractor/noncopyable.h>
#include <tractor/SockAddr.h>
#include <tractor/Buffer.h>

#include <boost/scoped_ptr.hpp>
#include <string>
#include <functional>
namespace tractor
{

    class EventLoop;
    class Channel;
    class Socket;

    // TcpConnection 封装一个Tcp连接
    class TcpConnection : public noncopyable,
                          public std::enable_shared_from_this<TcpConnection>
    {
    public:
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
        typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
        // typedef std::function<void(const TcpConnectionPtr &, const char *data, ssize_t len)> MessageCallback;
        typedef std::function<void(const TcpConnectionPtr &, Buffer *data, int64_t)> MessageCallback;

        TcpConnection();
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const SockAddr &localAddr,
                      const SockAddr &peerAddr);
        ~TcpConnection();
        EventLoop *getLoop() const { return loop_; }
        const std::string &name() const { return name_; }

        const SockAddr &localAddress() { return localAddr_; }
        const SockAddr &peerAddress() { return peerAddr_; }

        bool connected() const { return state_ == kConnected; }

        void connectEstablished();
        void connectDestroyed();
        void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = cb; }
        void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; }
        void setCloseCallback(CloseCallback cb) { closeCallback_ = cb; }

        void send(const std::string &message);
        void shutdown();

    private:
        enum ConnectState_
        {
            kConnecting,
            kConnected,
            kDisconnected,
            kDisconnecting
        };

        void setState_(ConnectState_ s) { state_ = s; };
        void handleRead_(int64_t);
        void handleWrite_();
        void handleClose_();
        void handleError_();

        void sendInLoop(const std::string &message);
        void shutdownInLoop();

        EventLoop *loop_;
        std::string name_;
        ConnectState_ state_;

        SockAddr localAddr_;
        SockAddr peerAddr_;

        boost::scoped_ptr<Channel> channel_;
        boost::scoped_ptr<Socket> socket_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;
    };
}
#endif