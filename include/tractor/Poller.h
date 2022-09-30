#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
// #include "EventLoop.h"
#include <vector>
#include <map>
struct pollfd;

namespace tractor
{
    class Channel;
    class Poller : public noncopyable
    {
    public:
        typedef std::vector<Channel *> ChannelList;

        // Poller(EventLoop *loop);
        Poller();

        ~Poller();

        void poll(int timeoutMs, ChannelList *activeChannels);

        void updateChannel(Channel *channel);
        // TODO 判断是否在对应线程
        // void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

    private:
        void fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;

        // EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };
}
#endif