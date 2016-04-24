#ifndef NET_POLLER_H_
#define NET_POLLER_H_

#include <tr1/memory>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include "noncopyable.h"

namespace net {
 
class Channel;
class EventLoop;

class Poller
{
 public:
	typedef std::vector<Channel*> ChannelList;
	typedef std::vector<struct epoll_event> EventList;
	typedef std::map<int, Channel*> ChannelMap;

	Poller();
	~Poller();

	void poll(int timeout_ms, ChannelList* active_channels);

	void update_channel(Channel* channel);

	void remove_channel(Channel* channel);

	const ChannelMap& channels() const {
		return _channels;
	}
 private:
 	void control(int operation, Channel* channel);

 	int _epollfd;
 	ChannelMap _channels;
 	EventList _events;
	DISALLOW_COPY_AND_ASSIGN(Poller);
};

}

#endif
