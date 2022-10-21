#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

#include <tractor/noncopyable.h>
#include <tractor/EventLoop.h>

namespace tractor
{

    class Channel : public noncopyable
    {
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void(int64_t)> ReadEventCallback;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        void handleEvent(int64_t receiveTime);

        void setReadCallback(const ReadEventCallback &cb)
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
        void setCloseCallback(const EventCallback &cb)
        {
            closeCallback_ = cb;
        }

        int getFd() const { return fd_; }
        int getEvents() const { return events_; }
        void setRevents(int revt) { revents_ = revt; }

        void enableReading()
        {
            events_ |= kReadEvent;
            update();
        }
        void enableWriting()
        {
            events_ |= kWriteEvent;
            update();
        }
        void disableWriting()
        {
            events_ &= ~kWriteEvent;
            update();
        }
        void disableAll()
        {
            events_ = kNoneEvent;
            update();
        }

        bool isNoneEvent() const { return events_ == kNoneEvent; }
        bool isWriting() const { return events_ & kWriteEvent; }
        int getIndex() { return index_; }
        void setIndex(int idx) { index_ = idx; }

        EventLoop *getOwnerLoop() { return loop_; }

    private:
        void update();

        bool eventHandleing_;

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;  // events_是它关心的IO事件， 由用户设置
        int revents_; // 目前活动的事件，由EventLoop/Poller设置
        int index_;   // used by Poller.

        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
        EventCallback closeCallback_;
    };

}
#endif