// Copyright [2012-2014] <HRG>
#ifndef NET_POLLER_H_
#define NET_POLLER_H_

#include <tr1/memory>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include "common/noncopyable.h"

 namespace net {

class Channel;
class EventLoop;

class Poller {
 public:
  typedef std::vector<Channel* > ChannelList;
  typedef std::vector<struct epoll_event> EventList;
  typedef std::map<int, Channel*> ChannelMap;

  Poller();
  ~Poller();

  void poll(int timeout_ms, ChannelList* active_channels);

  // ADD if channel not already in poller, else MOD
  // Channel **MUST** has events to waited in poller
  void update_channel(Channel* channel);

  // Remove channel from poller
  void remove_channel(Channel* channel);

  // For unittest only
  const ChannelMap& channels() const {
    return channels_;
  }

 private:
  // performs control operations
  void control(int operation, Channel* channel);

  int epollfd_;
  ChannelMap channels_;
  EventList events_;
  DISALLOW_COPY_AND_ASSIGN(Poller);
};

}  // namespace net

#endif  // NET_POLLER_H_
