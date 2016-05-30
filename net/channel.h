// Copyright [2012-2014] <HRG>
#ifndef NET_CHANNEL_H_
#define NET_CHANNEL_H_

#include <tr1/memory>
#include <sys/epoll.h>
#include <stdint.h>
#include <stdio.h>
#include <tr1/functional>
#include "common/noncopyable.h"

namespace hrg { namespace net {

class Poller;

const uint32_t kNoneEvent = 0;
const uint32_t kReadEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
const uint32_t kWriteEvent = EPOLLOUT;

class Channel: public std::tr1::enable_shared_from_this<Channel> {
 public:
  typedef std::tr1::function<void()> EventCallback;

  Channel(std::tr1::shared_ptr<Poller> poller, int fd);
  ~Channel();

  void set_read_callback(const EventCallback&);
  void set_write_callback(const EventCallback&);
  void set_close_callback(const EventCallback&);
  void set_error_callback(const EventCallback&);

  // Tie this channel to the owner object managed by shared_ptr,
  // prevent the owner object being destroyed in handleEvent.
  void tie(const std::tr1::shared_ptr<void>& owner);

  void enable_reading();

  // set reterned events, will be invoked by poller
  void set_revents(uint32_t revents);

  void handle_event();

  bool has_waited_event() const;

  bool is_writing() const;
  void enable_writing();
  void disable_writing();

  // Be invoked when socket fd will be removed from poller
  void disable_all();

  int fd() const;
  uint32_t events() const;

 private:
  void handle_event_with_guard();

  std::tr1::shared_ptr<Poller> poller_;
  const int fd_;
  uint32_t events_;   // Events that we wait
  uint32_t revents_;  // Events that returned by epoll_wait
  std::tr1::weak_ptr<void> tie_;
  bool tied_;

  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
  DISALLOW_COPY_AND_ASSIGN(Channel);
};

}  // namespace net
}  // namespace hrg
#endif  // NET_CHANNEL_H_
