#include <sys/timerfd.h>
#include <cstring>

#include "tractor/EventLoop.h"
#include "tractor/Channel.h"

using namespace std;

tractor::EventLoop *gloop;

void threadFunction()
{
    printf("Timeout!\n");
    gloop->quit();
}
int main()
{
    tractor::EventLoop loop;
    gloop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    tractor::Channel channel(&loop, timerfd);
    channel.setReadCallback(threadFunction);
    channel.enableReading();
    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();

    ::close(timerfd);
    return 0;
}