#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <set>
#include <vector>
#include <functional>

#include <tractor/Channel.h>
#include <tractor/noncopyable.h>
namespace tractor

{

  class EventLoop;
  class Timer;

  ///
  /// A best efforts timer queue.
  /// No guarantee that the callback will be on time.
  ///
  class TimerQueue : public noncopyable
  {
  public:
    typedef std::function<void()> TimerCallback;
    typedef std::shared_ptr<Timer> TimerPtr;
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerPtr addTimer(const TimerCallback &cb,
                      int64_t when,
                      double interval);

    // void cancel(TimerId timerId);

  private:
    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    typedef std::pair<int64_t, Timer *> Entry;
    typedef std::set<Entry> TimerList;

    void addTimerInLoop(TimerPtr timer);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(int64_t now);
    void reset(const std::vector<Entry> &expired, int64_t now);

    bool insert(Timer *timer);

    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;
  };

}
#endif // MUDUO_NET_TIMERQUEUE_H
