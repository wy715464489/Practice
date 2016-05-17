#include "eventloop.h"
#include "channel.h"
#include "poller.h"
#include "socket_ops.h"

namespace net
{
	const int kDefaultPollTimeoutMs = 10;

	time_t CalculateTimer(const timeval& tv) 
	{
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}

	EventLoop::EventLoop():
	_quit(false),
	_update_interval(0)
	{}

	void EventLoop::set_update(EventLoopUpdate update, int update_interval)
	{
		_update = update;
		_update_interval = update_interval;
	}

	void EventLoop::loop() 
	{
		timeval prev;
		gettimeofday(&prev, NULL);

		_quit = false;

		while(!_quit) {
			_poller->poll(kDefaultPollTimeoutMs, &_active_channels);

			for (ChannelList::iterator it = _active_channels.begin();
 				it != _active_channels.end(); ++it) {
				// Invoke active channel's event handler
				Channel* current_active_channel = *it;
				current_active_channel->handle_event();
			}
			if(_update) {
				timeval curr;
				gettimeofday(&curr, NULL);
				const int elapsed_ms = (curr.tv_sec - prev.tv_sec) * 1000
						   + (curr.tv_usec - prev.tv_usec) / 1000;
				if (elapsed_ms <= -1000 || elapsed_ms >= _update_interval) {
					gettimeofday(&prev, NULL);
					_update();
				}
			}
		}

		// Check timer queue
		while(!_timer_queue.empty())
		{
			timeval curr;
			gettimeofday(&curr, NULL);
			Timer timer = _timer_queue.top();
			if(CalculateTimer(curr) >= timer.timer)
			{
				timer.cb();
				_timer_queue.pop();
			}
			else
			{
				break;
			}
		}
	}
	
	void EventLoop::quit()
	{
		_quit = true;
	}


	void EventLoop::run_after(int time_ms, TimerCallback cb)
	{
		timeval curr;
		gettimeofday(&curr, NULL);
		Timer timer;
		timer.timer = CalculateTimer(curr) + time_ms;
		timer.cb = cb;
		_timer_queue.push(timer);
	}

	std::tr1::shared_ptr<Poller> EventLoop::poller() {
  		return _poller;
  	}
}

