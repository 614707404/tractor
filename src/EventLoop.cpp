#include <tractor/Channel.h>
#include <tractor/EventLoop.h>

#include <iostream>
#include <assert.h>
#include <poll.h>
using namespace tractor;

__thread EventLoop *t_loopInThisThread = 0;

EventLoop::EventLoop()
    : looping_(false),
      threadId_(static_cast<pid_t>(::syscall(SYS_gettid)))
{
    std::cout << "EventLoop created " << this << " in thread " << threadId_ << std::endl;
    if (t_loopInThisThread)
    {
        std::cout << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_ << std::endl;
    }
    else
    {
        t_loopInThisThread = this;
    }
}
EventLoop::~EventLoop()
{
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
        poller_->poll(0, &activeChannels_);
        for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent();
        }
    }

    std::cout << "EventLoop " << this << " stop looping " << std::endl;
    looping_ = false;
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
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