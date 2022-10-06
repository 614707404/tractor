#include <tractor/Timer.h>
#include <sys/time.h>
using namespace tractor;

void Timer::restart(int64_t now)
{
  if (repeat_)
  {
    int64_t delta = static_cast<int64_t>(interval_ * kMicroSecondsPerSecond);
    expiration_ = now + delta;
  }
  else
  {
    expiration_ = 0;
  }
}
// TODO to_string(now()) 存在溢出问题
int64_t Timer::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return seconds * kMicroSecondsPerSecond + tv.tv_usec;
}
