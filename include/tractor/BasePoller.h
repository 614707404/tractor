#ifndef BASEPOLLER_H
#define BASEPOLLER_H

#include <tractor/noncopyable.h>
#include <tractor/EventLoop.h>
#include <tractor/Channel.h>

#include <vector>
#include <map>

namespace tractor
{

    class BasePoller : public noncopyable
    {
    public:
        typedef std::vector<Channel *> ChannelList;

        BasePoller(EventLoop *loop) : ownerLoop_(loop){};
        ~BasePoller() = default;

        virtual int64_t poll(int timeoutMs, ChannelList *activeChannels) = 0;
        virtual void updateChannel(Channel *channel) = 0;
        virtual void removeChannel(Channel *channel) = 0;

        virtual bool hasChannel(Channel *channel) const
        {
            assertInLoopThread();
            ChannelMap::const_iterator it = channels_.find(channel->getFd());
            return it != channels_.end() && it->second == channel;
        }

        // static BasePoller *newDefaultPoller(EventLoop *loop);

        void assertInLoopThread() const
        {
            ownerLoop_->assertInLoopThread();
        }

    protected:
        // key: fd , value: channel ptr
        typedef std::map<int, Channel *> ChannelMap;
        ChannelMap channels_;

    private:
        EventLoop *ownerLoop_;
    };
}
#endif