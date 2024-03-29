#include <tractor/Poller.h>
#include <tractor/Channel.h>
#include <tractor/Timer.h>

#include <iostream>
#include <poll.h>
#include <assert.h>

using namespace tractor;

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop) {}
// Poller::Poller() {}
Poller::~Poller() {}

int64_t Poller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // XXX pollfds_ shouldn't change
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    int now = Timer::now();
    if (numEvents > 0)
    {
        std::cout << numEvents << " events happended" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        std::cout << " nothing happended" << std::endl;
    }
    else
    {
        std::cout << "ERROR : Poller::poll()" << std::endl;
    }
    return now;
}
void Poller::fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin();
         pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->getFd() == pfd->fd);
            channel->setRevents(pfd->revents);
            // pfd->revents = 0;
            activeChannels->push_back(channel);
        }
    }
}
void Poller::updateChannel(Channel *channel)
{

    std::cout << "fd = " << channel->getFd() << " events = " << channel->getEvents() << std::endl;
    if (channel->getIndex() < 0)
    {
        // a new one, add to pollfds_
        assert(channels_.find(channel->getFd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->getFd();
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(idx); // 记录索引
        channels_[pfd.fd] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.find(channel->getFd()) != channels_.end());
        assert(channels_[channel->getFd()] == channel);
        int idx = channel->getIndex();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->getFd() || pfd.fd == -channel->getFd() - 1);
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            // ignore this pollfd
            pfd.fd = -channel->getFd() - 1;
        }
    }
}
void Poller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    std::cout << "fd = " << channel->getFd() << std::endl;
    assert(channels_.find(channel->getFd()) != channels_.end());
    assert(channels_[channel->getFd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->getIndex();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx];
    (void)pfd;
    assert(pfd.fd == -channel->getFd() - 1 && pfd.events == channel->getEvents());
    size_t n = channels_.erase(channel->getFd());
    assert(n == 1);
    (void)n;
    if (static_cast<size_t>(idx) == pollfds_.size() - 1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->setIndex(idx);
        pollfds_.pop_back();
    }
}