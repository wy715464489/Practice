// Copyright [2012-2014] <HRG>
#include "net/eventloop.h"
#include "net/channel.h"
#include "net/poller.h"
#include "net/socket_ops.h"
#include "common/log.h"
using hrg::common::INFO_LOG;

namespace hrg { namespace net {

const int kDefaultPollTimeoutMs = 10;

time_t CalculateTimer(const timeval& tv) {
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

EventLoop::EventLoop()
  : poller_(new Poller()),
    quit_(false),
    update_interval_(0) {
}

void EventLoop::set_update(EventLoopUpdate update, int update_interval) {
  update_ = update;
  update_interval_ = update_interval;
}

void EventLoop::loop() {
  timeval prev;
  gettimeofday(&prev, NULL);

  // unittest may invoke quit() many times
  quit_ = false;

  while (!quit_) {
    poller_->poll(kDefaultPollTimeoutMs, &active_channels_);

    for (ChannelList::iterator it = active_channels_.begin();
        it != active_channels_.end(); ++it) {
        // Invoke active channel's event handler
        Channel* current_active_channel = *it;
        current_active_channel->handle_event();
    }

    // Check updater
    if (update_) {
      timeval curr;
      gettimeofday(&curr, NULL);
      const int elapsed_ms = (curr.tv_sec - prev.tv_sec) * 1000
                           + (curr.tv_usec - prev.tv_usec) / 1000;
      if (elapsed_ms <= -1000 || elapsed_ms >= update_interval_) {
	      if (elapsed_ms <= -1000) {
	        INFO_LOG("elpased_ms:%d is less than -1000, function:EventLoop::loop\n", elapsed_ms);
	      }
        gettimeofday(&prev, NULL);
        update_();
      }
    }

    // Check timer queue
    while (!timer_queue_.empty()) {
      timeval curr;
      gettimeofday(&curr, NULL);
      Timer timer = timer_queue_.top();
      if (CalculateTimer(curr) >= timer.timer) {
        timer.cb();
        timer_queue_.pop();
      } else {
        break;
      }
    }
  }
}

void EventLoop::quit() {
  quit_ = true;
}

void EventLoop::run_after(int time_ms, TimerCallback cb) {
    timeval curr;
    gettimeofday(&curr, NULL);
    Timer timer;
    timer.timer = CalculateTimer(curr) + time_ms;
    timer.cb = cb;
    timer_queue_.push(timer);
}

std::tr1::shared_ptr<Poller> EventLoop::poller() {
  return poller_;
}

}  // namespace net
}  // namespace hrg
