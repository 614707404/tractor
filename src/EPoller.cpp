#include <sys/epoll.h>
#include <iostream>
#include <cstring>

#include <tractor/EPoller.h>
#include <tractor/Timer.h>

using namespace tractor;
namespace // 匿名命名空间，效果与static相同，作用域为当前文件
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}
EPoller::EPoller(EventLoop *loop)
    : BasePoller(loop),
      eventfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (eventfd_ < 0)
    {
        std::cout << "EPoller::EPoller eventfd_ < 0" << std::endl;
    }
}
EPoller::~EPoller()
{
    ::close(eventfd_);
}
int64_t EPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // wait for an I/O event on an epoll file descriptor
    int numEvents = ::epoll_wait(eventfd_,
                                 events_.data(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    int now = Timer::now();
    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        // LOG_TRACE << "nothing happened";
    }
    else
    {
        // error happens, log uncommon ones
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            // LOG_SYSERR << "EPollPoller::poll()";
        }
    }

    return now;
}
void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList *activeChannels) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        int fd = channel->fd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
void EPoller::updateChannel(Channel *channel)
{
    BasePoller::assertInLoopThread();
    const int index = channel->index();
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        (void)fd; // ? TODO
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EPoller::removeChannel(Channel *channel)
{
    BasePoller::assertInLoopThread();
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}
void EPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    std::memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(eventfd_, operation, fd, &event) < 0)
    {
        std::cout << "epoll_ctl return < 0" << std::endl;
    }
}