// Copyright [2012-2014] <HRG>
#include "net/channel.h"
#include "net/poller.h"
#include "common/log.h"
using hrg::common::LogSystem;
using hrg::common::ERROR_LOG;

namespace hrg { namespace net {

void DefaultEventCallback() {
  ERROR_LOG("Default event callback invoked, "
            "you must set you own event callback in production\n");
}

Channel::Channel(std::tr1::shared_ptr<Poller> poller, int fd)
  : poller_(poller),
    fd_(fd),
    events_(0),
    revents_(0),
    tied_(false),
    read_callback_(DefaultEventCallback),
    write_callback_(DefaultEventCallback),
    close_callback_(DefaultEventCallback),
    error_callback_(DefaultEventCallback) {
}

Channel::~Channel() {
  // fd_ maybe invalid since is ownd by tcpconnection's socket
  // so should not call disable_all() at this moment;
}

void Channel::set_read_callback(const EventCallback& cb) {
  read_callback_ = cb;
}

void Channel::set_write_callback(const EventCallback& cb) {
  write_callback_ = cb;
}

void Channel::set_close_callback(const EventCallback& cb) {
  close_callback_ = cb;
}

void Channel::set_error_callback(const EventCallback& cb) {
  error_callback_ = cb;
}

void Channel::tie(const std::tr1::shared_ptr<void>& owner) {
  tie_ = owner;
  tied_ = true;
}

void Channel::enable_reading() {
  events_ |= kReadEvent;
  poller_->update_channel(this);
}

void Channel::set_revents(uint32_t revents) {
  revents_ = revents;
}

void Channel::handle_event() {
  // This channel maybe deleted in the callbacks
  // for example, an error occurs in read_callback_()
  if (tied_) {
    std::tr1::shared_ptr<void> guard = tie_.lock();
    if (guard != NULL) {
      handle_event_with_guard();
    } else {
      ERROR_LOG("Channel::handle_event's guard is NULL, fd=%d\n", fd_);
    }
  } else {
    handle_event_with_guard();
  }
}

void Channel::handle_event_with_guard() {
  // libevent's epoll.c
  /*
    int what = events[i].events;
    short ev = 0;

    if (what & (EPOLLHUP|EPOLLERR)) {
      ev = EV_READ | EV_WRITE;
    } else {
      if (what & EPOLLIN)
        ev |= EV_READ;
      if (what & EPOLLOUT)
        ev |= EV_WRITE;
    }
  */
  // Please man epoll_ctl to check epoll events
  // Maybe only EV_READ and EV_WRITE is much simpler
  if (revents_ & EPOLLERR) {
    error_callback_();
  } else if (revents_ & EPOLLHUP) {
    close_callback_();
  } else {
    if (revents_ & kReadEvent) {
      read_callback_();
    }
    if (revents_ & kWriteEvent) {
      write_callback_();
    }
  }
}

bool Channel::has_waited_event() const {
  return events_ != kNoneEvent;
}

bool Channel::is_writing() const {
  return events_ & kWriteEvent;
}

void Channel::enable_writing() {
  events_ |= kWriteEvent;
  poller_->update_channel(this);
}

void Channel::disable_writing() {
  events_ &= ~kWriteEvent;
  poller_->update_channel(this);
}

int Channel::fd() const {
  return fd_;
}

uint32_t Channel::events() const {
  return events_;
}

void Channel::disable_all() {
  revents_ = kNoneEvent;
  events_ = kNoneEvent;
  poller_->remove_channel(this);
}

}  // namespace net
}  // namespace hrg
