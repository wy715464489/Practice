// Copyright [2012-2014] <HRG>
#ifndef NET_EVENTLOOP_H_
#define NET_EVENTLOOP_H_

#include <stdint.h>
#include <sys/time.h>
#include <tr1/memory>
#include <tr1/functional>
#include <vector>
#include <queue>
#include "common/noncopyable.h"

 namespace net {

class Channel;
class Poller;

typedef std::tr1::function<void ()> EventLoopUpdate;
typedef std::tr1::function<void ()> TimerCallback;

struct Timer {
  time_t  timer;
  TimerCallback cb;
};

struct TimerComparer {
  bool operator()(const Timer& lhs, const Timer& rhs) {
    return lhs.timer > rhs.timer;
  }
};

// In milliseconds
time_t CalculateTimer(const timeval& tv);

class EventLoop {
 public:
  static EventLoop& instance() {
    static EventLoop loop;
    return loop;
  }

  // EventLoopUpdate will be invoked every update_interval ms
  // Mininal update_interval should >= kDefaultPollTimeoutMs
  void set_update(EventLoopUpdate update, int update_interval);

  // Will loops forever, must be the last function in main()
  // Will be exit in the next run when call quit()
  void loop();

  // Loop() will exit in the next run
  void quit();

  void run_after(int time_ms, TimerCallback cb);

  std::tr1::shared_ptr<Poller> poller();

  // For unittest only
  size_t timer_queue_size() const {
    return timer_queue_.size();
  }

 private:
  EventLoop();

  typedef std::vector<Channel*> ChannelList;
  ChannelList active_channels_;
  std::tr1::shared_ptr<Poller> poller_;
  bool quit_;  // If set, loop() will be exit in the next run
  EventLoopUpdate update_;  // will be invoked every update_interval ms
  int update_interval_;  // in ms
  typedef std::priority_queue<Timer, std::vector<Timer>, TimerComparer>
          TimerQueue;
  TimerQueue timer_queue_;

  DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

}  // namespace net

#endif  // NET_EVENTLOOP_H_
