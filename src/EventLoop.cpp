#include <tractor/Channel.h>
#include <tractor/EventLoop.h>
#include <tractor/Poller.h>
#include <tractor/EPoller.h>
#include <tractor/Timer.h>
#include <tractor/TimerQueue.h>

#include <iostream>
#include <spdlog/spdlog.h>

#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <sys/time.h>
using namespace tractor;

namespace
{
    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };
    IgnoreSigPipe ignoreSigPipe;
    __thread EventLoop *t_loopInThisThread = 0;
    const int kPollTimeMs = 10000;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(static_cast<pid_t>(::syscall(SYS_gettid))),
      callingPendingFunctors_(false),
      //   poller_(new Poller(this)),
      poller_(new EPoller(this)),
      //   timerQueue_(new TimerQueue(this)),
      //   wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    // spdlog::debug("EventLoop created {0} in thread {1}", this, threadId_);

    if (t_loopInThisThread)
    {
        // spdlog::error("Another EventLoop {0} exists in this thread {1}", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
}
EventLoop::~EventLoop()
{
    // spdlog::debug("EventLoop {0} of thread {1} destructs in thread", this, threadId_, static_cast<pid_t>(::syscall(SYS_gettid)));

    assert(!looping_);
    t_loopInThisThread = NULL;
}
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }
    // spdlog::trace("EventLoop {} stop looping", this);

    looping_ = false;
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread_()
{
    std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << static_cast<pid_t>(::syscall(SYS_gettid)) << std::endl;
}
void EventLoop::quit()
{
    quit_ = true;
}
void EventLoop::runInLoop(const Functor &cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(const Functor &cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}
void EventLoop::runAt(const int64_t &time, const Functor &cb)
{
    timerQueue_->addTimer(cb, time, 0.0);
}

void EventLoop::runAfter(double delay, const Functor &cb)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    int64_t currTime = seconds * 1e6 + tv.tv_usec;
    int64_t time = currTime + static_cast<int64_t>(delay * 1e6);
    runAt(time, cb);
}

void EventLoop::runEvery(double interval, const Functor &cb)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    int64_t currTime = seconds * 1e6 + tv.tv_usec;
    int64_t time = currTime + static_cast<int64_t>(interval * 1e6);
    timerQueue_->addTimer(cb, time, interval);
}
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}
