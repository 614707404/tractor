#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
#include "EventLoop.h"
#include <vector>
#include <map>
struct pollfd;
/*
    POLLIN There is data to read.
    POLLPRI  There is some exceptional condition on the file descriptor.
    POLLOUT Writing is now possible
    POLLHUP Stream socket peer closed connection
    POLLNVAL Invalid request
*/
namespace tractor
{
    class Channel;
    class Poller : public noncopyable
    {
    public:
        typedef std::vector<Channel *> ChannelList;

        Poller(EventLoop *loop);
        // Poller();

        ~Poller();

        int64_t poll(int timeoutMs, ChannelList *activeChannels);

        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        // TODO 判断是否在对应线程
        void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

    private:
        void fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };
}
#endif