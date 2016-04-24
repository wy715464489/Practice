#include "channel.h"

namespace net {

void DefaultEventCallback() {

}

Channel::Channel(std::tr1::shared_ptr<Poller> poller, int fd) 
	:	_poller(poller),
		_fd(fd),
		_events(0),
		_revents(0),
		_tied(false),
		_read_callback(DefaultEventCallback),
		_write_callback(DefaultEventCallback),
		_close_callback(DefaultEventCallback),
		_error_callback(DefaultEventCallback) {
}

Channel::~Channel() {

}

void Channel::set_read_callback(const EventCallback& cb) {
  _read_callback = cb;
}

void Channel::set_write_callback(const EventCallback& cb) {
  _write_callback = cb;
}

void Channel::set_close_callback(const EventCallback& cb) {
  _close_callback = cb;
}

void Channel::set_error_callback(const EventCallback& cb) {
  _error_callback = cb;
}

void Channel::tie(const std::tr1::shared_ptr<void>& owner) {
	_tie = owner;
	_tied = true;
}

void Channel::enable_reading() {
	_events |= kReadEvent;
}

void Channel::set_revents(uint32_t revents) {
	_revents = revents;
}

void Channel::handle_event() {
	if(_tied) {
		std::tr1::shared_ptr<void> guard = _tie.lock();
		if(guard != NULL) {
			handle_event_with_guard();
		}
	} else {
		handle_event_with_guard();
	}
}

void Channel::handle_event_with_guard() {
	if(_revents & EPOLLERR) {
		_error_callback();
	} else if(_revents & EPOLLHUP) {
		_close_callback();
	} else {
		if(_revents & kReadEvent) {
			_read_callback();
		}
		if(_revents & kWriteEvent) {
			_write_callback();
		}
	}
}

bool Channel::has_waited_event() const {
	return _events != kNoneEvent;
}

bool Channel::is_writing() const {
	return _events & kWriteEvent;
}

void Channel::enable_writing() {
	_events |= kWriteEvent;
}

void Channel::disable_writing() {
	_events &= ~kWriteEvent;
}

int Channel::fd() const {
	return _fd;
}

uint32_t Channel::events() const {
	return _events;
}

void Channel::disable_all() {
	_revents = kNoneEvent;
	_events = kNoneEvent;
}

}