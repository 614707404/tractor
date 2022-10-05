#include <tractor/TcpConnection.h>
#include <tractor/Channel.h>
#include <tractor/Socket.h>
#include <tractor/EventLoop.h>

#include <iostream>
using namespace tractor;
using namespace std;

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const SockAddr &localAddr,
                             const SockAddr &peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      channel_(new Channel(loop, sockfd)),
      socket_(new Socket(sockfd))
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead_, this));
}
TcpConnection::~TcpConnection()
{
    cout << "TcpConnection::dtor[" << name_ << "] at " << this
         << " fd=" << channel_->fd() << endl;
}
void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState_(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead_()
{
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    messageCallback_(shared_from_this(), buf, n);
    // FIXME: close connection if n == 0
}
