#include "poller.h"
#include "channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace net {

const int kInitEventListSize = 200;

Poller::Poller()
	:	_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
		_events(kInitEventListSize) {
	if(_epollfd < 0){
		exit(1);
	}
}

Poller::~Poller() {
	::close(_epollfd);
}

void Poller::poll(int timeout_ms,ChannelList* active_channels) {
	active_channels->clear();
	int num_events = ::epoll_wait(_epollfd,
															&*_events.begin(),
															static_cast<int>(_events.size()),
															timeout_ms);
	if(num_events > 0) {
		for(int i = 0; i < num_events; i++) {
			Channel* channel = reinterpret_cast<Channel*>(_events[i].data.ptr);
			channel->set_revents(_events[i].events);
			active_channels->push_back(channel);
		}

		if(static_cast<size_t>(num_events) == _events.size()) {
			_events.resize(_events.size() * 2);
		}
	} else if (num_events == 0) {
		if(errno != EINTR) {
			exit(1);
		}
	}
}

void Poller::update_channel(Channel* channel) {
	const int fd = channel->fd();

	if(!channel->has_waited_event()) {
		exit(1);
	}

	if(_channels.find(fd) == _channels.end()) {
		control(EPOLL_CTL_ADD, channel);
		_channels.insert(std::make_pair(fd, channel));
	} else {
		control(EPOLL_CTL_MOD, channel);
	}
}

void Poller::remove_channel(Channel* channel) {
	const int fd = channel->fd();

	if(_channels.find(fd) != _channels.end()) {
		control(EPOLL_CTL_DEL, channel);
		_channels.erase(fd);
	}
}

void Poller::control(int operation, Channel* channel) {
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	const int fd = channel->fd();
	if(::epoll_ctl(_epollfd, operation, fd, &event) < 0) {

	}
}

}