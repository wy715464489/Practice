// Copyright [2012-2014] <HRG>
#include "net/poller.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/channel.h"
#include "common/log.h"
using hrg::common::LogSystem;
using hrg::common::ERROR_LOG;

namespace hrg { namespace net {

const int kInitEventListSize = 200;

Poller::Poller()
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    ERROR_LOG("Failed to execute epoll_create1, reason:%s\n",
              strerror(errno));
    exit(1);
  }
}

Poller::~Poller() {
  ::close(epollfd_);
}

void Poller::poll(int timeout_ms, ChannelList* active_channels) {
  active_channels->clear();
  int num_events = ::epoll_wait(epollfd_,
                              &*events_.begin(),
                              static_cast<int>(events_.size()),
                              timeout_ms);
  if (num_events > 0) {
    // Fill active channels
    for (int i = 0; i < num_events; i++) {
      Channel* channel = reinterpret_cast<Channel*>(events_[i].data.ptr);
      channel->set_revents(events_[i].events);
      active_channels->push_back(channel);
    }
    // Check if needs to enlarge events list;
    if (static_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
    // Timeout, No events occur
    // DEBUG_LOG("No event occurs, epoll timeout in %d
    // milliseconds\n", timeout_ms);
  } else {
    if (errno != EINTR) {
      ERROR_LOG("Epoll wait error, reason:%s\n", strerror(errno));
      exit(1);
    }
  }
}

void Poller::update_channel(Channel* channel) {
  const int fd = channel->fd();
  // DEBUG_LOG("update channel in poller: fd = %d, events:%u\n",
  //        fd, channel->events());

  if (!channel->has_waited_event()) {
    ERROR_LOG("Operation update channel must has waited_events, fd = %d\n",
              fd);
    exit(1);  // Must not happen, for debug purpose
  }

  if (channels_.find(fd) == channels_.end()) {
    control(EPOLL_CTL_ADD, channel);
    channels_.insert(std::make_pair(fd, channel));
  } else {
    control(EPOLL_CTL_MOD, channel);
  }
}

void Poller::remove_channel(Channel* channel) {
  const int fd = channel->fd();
  // DEBUG_LOG("remove channel in poller: fd = %d, events:%u\n",
  //         fd, channel->events());
  if (channels_.find(fd) != channels_.end()) {
    control(EPOLL_CTL_DEL, channel);
    channels_.erase(fd);
  }
}

void Poller::control(int operation, Channel* channel) {
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  const int fd = channel->fd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    ERROR_LOG("epoll_ctl error: fd=%d, events=%u, reason=%s\n",
              fd, channel->events(), strerror(errno));
  }
}

}  // namespace net
}  // namespace hrg
