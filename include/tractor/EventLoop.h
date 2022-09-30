#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "noncopyable.h"
#include "Poller.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <vector>
// #include <boost/scoped_ptr.hpp>
#include <memory>

namespace tractor
{
    class Channel;

    class EventLoop : public noncopyable
    {
    public:
        EventLoop();
        ~EventLoop();
        void loop();
        void quit();
        void updateChannel(Channel *channel);
        bool isInLoopThread() const { return threadId_ == static_cast<pid_t>(::syscall(SYS_gettid)); }
        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread_();
            }
        }

    private:
        void abortNotInLoopThread_();
        typedef std::vector<Channel *> ChannelList;
        bool looping_;
        bool quit_;
        const pid_t threadId_;
        // EventLoop 直接拥有 Poller
        std::unique_ptr<Poller> poller_;

        ChannelList activeChannels_;
    };

}
#endif