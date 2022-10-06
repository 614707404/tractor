#include <tractor/TcpConnection.h>
#include <tractor/Channel.h>
#include <tractor/Socket.h>
#include <tractor/EventLoop.h>

#include <string.h>
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
        std::bind(&TcpConnection::handleRead_, this, std::placeholders::_1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite_, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose_, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError_, this));
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

void TcpConnection::handleRead_(int64_t receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose_();
    }
    else
    {
        errno = savedErrno;
        std::cout << "TcpConnection::handleRead" << std::endl;
        handleError_();
    }
    // FIXME: close connection if n == 0
}
void TcpConnection::handleClose_()
{
    loop_->assertInLoopThread();
    cout << "TcpConnection::handleClose state = " << state_ << endl;
    assert(state_ == kConnected);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError_()
{
    int err;

    socklen_t errlen = sizeof err;

    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
    {
        err = errno;
    }
    char t_errnobuf[512];
    cout << "TcpConnection::handleError [" << name_
         << "] - SO_ERROR = " << err << " " << strerror_r(err, t_errnobuf, sizeof t_errnobuf) << endl;
}
void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    setState_(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(get_pointer(channel_));
}
void TcpConnection::handleWrite_() {}
