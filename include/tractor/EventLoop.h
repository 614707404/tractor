#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "noncopyable.h"
// #include "Poller.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <functional>
#include <mutex>

namespace tractor
{
    class Channel;
    class EPoller;
    class TimerQueue;
    class EventLoop : public noncopyable
    {
    public:
        typedef std::function<void()> Functor;
        EventLoop();
        ~EventLoop();
        void loop();
        void quit();

        void runInLoop(const Functor &cb);
        void runAt(const int64_t &time, const Functor &cb);
        void runAfter(double delay, const Functor &cb);
        void runEvery(double interval, const Functor &cb);

        void queueInLoop(const Functor &cb);
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        bool isInLoopThread() const { return threadId_ == static_cast<pid_t>(::syscall(SYS_gettid)); }
        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread_();
            }
        }

        void wakeup();

    private:
        void abortNotInLoopThread_();
        void handleRead(); // waked up
        void doPendingFunctors();

        typedef std::vector<Channel *> ChannelList;

        bool looping_;
        bool quit_;
        bool callingPendingFunctors_; /* atomic */

        const pid_t threadId_;
        int64_t pollReturnTime_;

        // EventLoop 直接拥有 Poller
        // std::unique_ptr<Poller> poller_;
        // boost::scoped_ptr<Poller> poller_;
        boost::scoped_ptr<EPoller> poller_;

        boost::scoped_ptr<TimerQueue> timerQueue_;
        int wakeupFd_;
        boost::scoped_ptr<Channel> wakeupChannel_;

        ChannelList activeChannels_;
        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_;
    };

}
#endif