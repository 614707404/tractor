#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <tractor/SockAddr.h>
#include <tractor/TcpConnection.h>
#include <tractor/noncopyable.h>

#include <boost/scoped_ptr.hpp>
#include <map>
#include <memory>
#include <functional>

namespace tractor
{
    class Acceptor;
    class EventLoop;

    // TcpServer 用于管理 TcpConnection
    class TcpServer : public noncopyable
    {
    public:
        TcpServer(EventLoop *loop, SockAddr &listenAddr);
        ~TcpServer();

        void start();
        void setConnectionCallback(const TcpConnection::ConnectionCallback &cb) { connectionCallback_ = cb; }
        void setMessageCallback(const TcpConnection::MessageCallback &cb) { messageCallback_ = cb; }
        void setCloseCallback(const TcpConnection::CloseCallback &cb) { closeCallback_ = cb; }

    private:
        typedef std::map<std::string, std::shared_ptr<TcpConnection>> ConnectionMap;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

        void newConnection(int sockfd, const SockAddr &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        EventLoop *loop_; // the acceptor loop
        const std::string name_;
        boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
        TcpConnection::ConnectionCallback connectionCallback_;
        TcpConnection::MessageCallback messageCallback_;
        TcpConnection::CloseCallback closeCallback_;

        bool started_;
        int nextConnId_; // always in loop thread
        ConnectionMap connections_;
    };
}
#endif