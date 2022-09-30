#include <tractor/Channel.h>
#include <poll.h>
#include <iostream>
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

void Channel::update()
{
    loop_->updateChannel(this);
}
void Channel::handleEvent()
{
    if (revents_ & POLLNVAL)
    {
        std::cout << "Channel::handle_event() POLLNVAL" << std::endl;
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_)
            readCallback_();
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
}