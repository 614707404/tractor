#include <tractor/Acceptor.h>
#include <tractor/EventLoop.h>
#include <iostream>
using namespace tractor;

Acceptor::Acceptor(EventLoop *loop, SockAddr &listenAddr)
    : loop_(loop),
      acceptSocket_(), // 创建一个新套接字
      acceptChannel_(loop, acceptSocket_.getFd()),
      listening_(false)
{
    acceptSocket_.Bind(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRand, this));
}
Acceptor::~Acceptor()
{
    // TODO 关闭 Channel
}
void Acceptor::Listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.Listen();
    acceptChannel_.enableReading();
}
void Acceptor::handleRand()
{
    loop_->assertInLoopThread();
    SockAddr peerAddr;
    int connFd = acceptSocket_.Accept(peerAddr);
    if (connFd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connFd, peerAddr);
        }
        else
        {
            ::close(connFd);
        }
    }
}
