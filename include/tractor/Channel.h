#ifndef CHANNEL_H
#define CHANNEL_H
// Channel 类简单封装文件描述符，方便 fd 的 IO 事件分发

#include <functional>
#include "noncopyable.h"
#include "EventLoop.h"
namespace tractor
{

    class Channel : public noncopyable
    {
    public:
        typedef std::function<void()> EventCallback;

        Channel(EventLoop *loop, int fd);
        void handleEvent();
        void setReadCallback(const EventCallback &cb)
        {
            readCallback_ = cb;
        }
        void setWriteCallback(const EventCallback &cb)
        {
            writeCallback_ = cb;
        }
        void setErrorCallback(const EventCallback &cb)
        {
            errorCallback_ = cb;
        }

        int fd() const { return fd_; }
        int events() const { return events_; }
        void set_revents(int revt) { revents_ = revt; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enableReading()
        {
            events_ |= kReadEvent;
            update();
        }
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        EventLoop *ownerLoop() { return loop_; }

    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;  // events_是它关心的IO事件， 由用户设置
        int revents_; // 目前活动的事件，由EventLoop/Poller设置
        int index_;   // used by Poller.

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
    };

}
#endif