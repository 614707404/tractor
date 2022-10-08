#ifndef EPOLLER_H
#define EPOLLER_H

#include <tractor/BasePoller.h>

#include <vector>

/* typedef union epoll_data
{
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event
{
    uint32_t events;   // Epoll events
    epoll_data_t data; // User data variable
}; */
namespace tractor
{
    class EPoller : public BasePoller
    {
    public:
        EPoller(EventLoop *);
        ~EPoller();
        int64_t poll(int timeoutMs, ChannelList *activeChannels) override;
        void updateChannel(Channel *channel) override;
        void removeChannel(Channel *channel) override;

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) const;

        typedef std::vector<struct epoll_event> EventList;
        void update(int operation, Channel *channel);

        int eventfd_;

        EventList events_;
    };
}
#endif