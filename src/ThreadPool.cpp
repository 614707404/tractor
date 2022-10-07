#include <tractor/ThreadPool.h>
#include <tractor/EventLoop.h>

#include <functional>
#include <assert.h>

using namespace tractor;
Thread::Thread()
    : loop_(NULL),
      exiting_(false),
      thread_(),
      mutex_(),
      condition_variable_() {}
Thread::~Thread()
{
    exiting_ = true;
    loop_->quit();
    if (thread_->joinable())
        thread_->join();
}

EventLoop *Thread::startLoop()
{
    // 到另一个线程中运行 threadFunc_
    thread_ = std::make_unique<std::thread>(std::bind(&Thread::threadFunc_, this));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == NULL)
            condition_variable_.wait(lock);
    }
    return loop_;
}

void Thread::threadFunc_()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        condition_variable_.notify_all();
    }
    loop_->loop();
}
ThreadPool::ThreadPool(EventLoop *baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0) {}
ThreadPool::~ThreadPool() {}

void ThreadPool::start()
{
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        Thread *t = new Thread();
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}
EventLoop *ThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}