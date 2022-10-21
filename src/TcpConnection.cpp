#include <tractor/TcpConnection.h>
#include <tractor/Channel.h>
#include <tractor/Socket.h>
#include <tractor/EventLoop.h>

#include <string.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <functional>
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
         << " fd=" << channel_->getFd() << endl;
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
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), &savedErrno);

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
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError_()
{
    int err;

    socklen_t errlen = sizeof err;

    if (::getsockopt(channel_->getFd(), SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
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
    assert(state_ == kConnected || state_ == kDisconnecting);

    setState_(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}
void TcpConnection::handleWrite_()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->getFd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                std::cout << "I am going to write more data" << std::endl;
            }
        }
        else
        {
            std::cout << "TcpConnection::handleWrite" << std::endl;
        }
    }
    else
    {
        std::cout << "Connection is down, no more writing" << std::endl;
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState_(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}
void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}
void TcpConnection::send(const std::string &message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}
void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->retrieveAsString());
            buf->retrieveAll();
        }
        else
        {
            // void (TcpConnection::*fp)(const StringPiece &message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop,
                          this, // FIXME
                          buf->retrieveAsString()));
            // std::forward<string>(message)));
        }
    }
}
void TcpConnection::sendInLoop(const std::string &message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->getFd(), message.data(), message.size());
        if (nwrote >= 0)
        {
            if (static_cast<size_t>(nwrote) < message.size())
            {
                std::cout << "I am going to write more data" << std::endl;
            }
            else if (writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                std::cout << "TcpConnection::sendInLoop" << std::endl;
            }
        }
    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size())
    {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}
// void TcpConnection::sendInLoop(const void *data, size_t len)
// {
//     loop_->assertInLoopThread();
//     ssize_t nwrote = 0;
//     size_t remaining = len;
//     bool faultError = false;

//     // if no thing in output queue, try writing directly
//     if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
//     {
//         nwrote = ::write(channel_->fd(), data, len);
//         if (nwrote >= 0)
//         {
//             remaining = len - nwrote;
//             if (remaining == 0 && writeCompleteCallback_)
//             {
//                 loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
//             }
//         }
//         else
//         {
//             nwrote = 0;
//             if (errno != EWOULDBLOCK)
//             {
//                 spdlog::error("TcpConnection::sendInLoop");
//                 if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
//                 {
//                     faultError = true;
//                 }
//             }
//         }
//     }

//     assert(remaining <= len);
//     if (!faultError && remaining > 0)
//     {
//         size_t oldLen = outputBuffer_.readableBytes();
//         if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
//         {
//             loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
//         }
//         outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
//         if (!channel_->isWriting())
//         {
//             channel_->enableWriting();
//         }
//     }
// }
void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}