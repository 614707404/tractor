// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef TIMER_H
#define TIMER_H

#include <tractor/noncopyable.h>
#include <functional>
namespace tractor
{

  class Timer : public noncopyable
  {
  public:
    typedef std::function<void()> TimerCallback;

    static int64_t now();
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    Timer(const TimerCallback &cb, int64_t when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0)
    {
    }

    void run() const
    {
      callback_();
    }

    int64_t getExpiration() const { return expiration_; }
    bool getRepeat() const { return repeat_; }

    void restart(int64_t now);

  private:
    const TimerCallback callback_;
    int64_t expiration_;
    const double interval_;
    const bool repeat_;
  };

}
#endif // MUDUO_NET_TIMER_H
