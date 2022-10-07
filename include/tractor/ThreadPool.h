#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <tractor/noncopyable.h>

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
namespace tractor
{
    class EventLoop;
    class Thread : public noncopyable
    {
    public:
        Thread();
        ~Thread();
        EventLoop *startLoop();

    private:
        void threadFunc_();

        EventLoop *loop_;
        bool exiting_;
        std::unique_ptr<std::thread> thread_;

        std::mutex mutex_;
        std::condition_variable condition_variable_;
    };
    class ThreadPool : public noncopyable
    {
    public:
        ThreadPool(EventLoop *baseLoop);
        ~ThreadPool();
        void setThreadNum(int numThreads) { numThreads_ = numThreads; }
        void start();
        EventLoop *getNextLoop();

    private:
        EventLoop *baseLoop_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<Thread *> threads_;
        std::vector<EventLoop *> loops_;
    };
}
#endif