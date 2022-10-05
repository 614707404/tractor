#include <sys/timerfd.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <memory>

#include <tractor/Timer.h>
#include <tractor/TimerQueue.h>

namespace tractor
{

  int createTimerfd()
  {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
      std::cout << "Failed in timerfd_create" << std::endl;
    }
    return timerfd;
  }

  struct timespec howMuchTimeFromNow(int64_t when)
  {
    int64_t microseconds = when - Timer::now();
    if (microseconds < 100)
    {
      microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timer::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timer::kMicroSecondsPerSecond) * 1000);
    return ts;
  }

  void readTimerfd(int timerfd, int64_t now)
  {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    std::cout << "TimerQueue::handleRead() " << howmany << " at " << std::to_string(now) << std::endl;
    if (n != sizeof howmany)
    {
      std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
    }
  }

  void resetTimerfd(int timerfd, int64_t expiration)
  {
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
      std::cout << "timerfd_settime()" << std::endl;
    }
  }
}

using namespace tractor;

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_()
{
  timerfdChannel_.setReadCallback(
      std::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  // TODO 改为智能指针
  for (TimerList::iterator it = timers_.begin();
       it != timers_.end(); ++it)
  {
    delete it->second;
  }
}

TimerQueue::TimerPtr TimerQueue::addTimer(const TimerCallback &cb,
                                          int64_t when,
                                          double interval)
{
  TimerPtr timer = std::make_shared<Timer>(cb, when, interval);
  loop_->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return timer;
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer.get());

  if (earliestChanged)
  {
    resetTimerfd(timerfd_, timer->getExpiration());
  }
}

void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  int64_t now = Timer::now();
  readTimerfd(timerfd_, now);

  std::vector<Entry> expired = getExpired(now);

  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin();
       it != expired.end(); ++it)
  {
    it->second->run();
  }

  reset(expired, now);
}
// TODO
std::vector<TimerQueue::Entry> TimerQueue::getExpired(int64_t now)
{
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  TimerList::iterator it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, int64_t now)
{
  int64_t nextExpire;

  for (std::vector<Entry>::const_iterator it = expired.begin();
       it != expired.end(); ++it)
  {
    if (it->second->getRepeat())
    {
      it->second->restart(now);
      insert(it->second);
    }
    else
    {
      // FIXME move to a free list
      delete it->second;
    }
  }

  if (!timers_.empty())
  {
    nextExpire = timers_.begin()->second->getExpiration();
  }

  if (nextExpire > 0)
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer *timer)
{
  bool earliestChanged = false;
  int64_t when = timer->getExpiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first)
  {
    earliestChanged = true;
  }
  std::pair<TimerList::iterator, bool> result =
      timers_.insert(std::make_pair(when, timer));
  assert(result.second);
  return earliestChanged;
}
