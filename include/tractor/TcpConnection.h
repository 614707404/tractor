#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <tractor/noncopyable.h>
#include <tractor/SockAddr.h>
#include <tractor/Buffer.h>

#include <boost/any.hpp>
#include <string>
#include <functional>
#include <memory>
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
        typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
        typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterMarkCallback;

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
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
        {
            highWaterMarkCallback_ = cb;
            highWaterMark_ = highWaterMark;
        }

        void send(const std::string &message);
        void send(Buffer *buf);
        void shutdown();

        void setTcpNoDelay(bool on);

        void setContext(const boost::any &context)
        {
            context_ = context;
        }
        boost::any *getContext()
        {
            return &context_;
        }

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
        // void sendInLoop(const void *data, size_t len);

        void shutdownInLoop();

        EventLoop *loop_;
        std::string name_;
        ConnectState_ state_;

        SockAddr localAddr_;
        SockAddr peerAddr_;

        std::unique_ptr<Channel> channel_;
        std::unique_ptr<Socket> socket_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;

        CloseCallback closeCallback_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;

        size_t highWaterMark_;

        boost::any context_;
    };
}
#endif