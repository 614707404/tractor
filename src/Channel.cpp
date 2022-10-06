#include <tractor/Channel.h>

#include <poll.h>
#include <iostream>
#include <assert.h>
using namespace tractor;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      events_(0),
      revents_(0),
      index_(-1) {}
Channel::~Channel()
{
    assert(!eventHandleing_);
}
void Channel::update()
{
    loop_->updateChannel(this);
}
void Channel::handleEvent(int64_t receiveTime)
{
    eventHandleing_ = true;
    if (revents_ & POLLNVAL) // Invalid request: fd not open
    {
        std::cout << "Channel::handle_event() POLLNVAL" << std::endl;
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        std::cout << "Channel::handle_event() POLLHUP" << std::endl;
        if (closeCallback_)
            closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_)
            readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
    eventHandleing_ = false;
}