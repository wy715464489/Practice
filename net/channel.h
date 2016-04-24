#ifndef NET_CHANNEL_H_
#define NET_CHANNEL_H_

#include <tr1/memory>
#include <sys/epoll.h>
#include <stdint.h>
#include <stdio.h>
#include <tr1/functional>
#include "noncopyable"

namespace net {

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
  
	void tie(const std::tr1::shared_ptr<void>& owner);

	void enable_reading();

	void set_revents(uint32_t revents);

	void handle_event();

	bool has_waited_event() const;

	bool is_writing() const;
	void enable_writing();
	void disable_writing();

	void disable_all();

	int fd() const;
	uint32_t events() const;

 private:
 	void handle_event_with_guard();

 	std::tr1::shared_ptr<Poller> _poller;
 	const int _fd;
 	uint32_t _events;
 	uint32_t _revents;
 	std::tr1::weak_ptr<void> _tie;
 	bool _tied;

 	EventCallback _read_callback;
  	EventCallback _write_callback;
  	EventCallback _close_callback;
  	EventCallback _error_callback;
  	DISALLOW_COPY_AND_ASSIGN(Channel);
};

}

#endif
