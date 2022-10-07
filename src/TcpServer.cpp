#include <tractor/TcpServer.h>
#include <tractor/EventLoop.h>
#include <tractor/Acceptor.h>

#include <iostream>
#include <string.h>
using namespace tractor;
TcpServer::TcpServer(EventLoop *loop, SockAddr &listenAddr)
    : loop_(loop),
      name_(listenAddr.toString()),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1),
      threadPool_(new ThreadPool(loop))
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}
TcpServer::~TcpServer()
{
}
void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
    }

    if (!acceptor_->Listening())
    {
        loop_->runInLoop(
            std::bind(&Acceptor::Listen, boost::get_pointer(acceptor_)));
    }
}
void TcpServer::newConnection(int sockfd, const SockAddr &peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    std::cout << "TcpServer::newConnection [" << name_
              << "] - new connection [" << connName
              << "] from " << peerAddr.toString() << std::endl;
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, (sockaddr *)(&localaddr), &addrlen) < 0)
    {
        std::cout << "ERROR sockets::getLocalAddr" << std::endl;
    }
    SockAddr localAddr(localaddr);
    EventLoop *ioLoop = threadPool_->getNextLoop();
    // FIXME poll with zero timeout to double confirm the new connection
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    std::cout << "TcpServer::removeConnectionInLoop [" << name_
              << "] - connection " << conn->name() << std::endl;
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    (void)n;
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}