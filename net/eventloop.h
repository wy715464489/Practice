#ifndef NET_EVENTLOOP_H_
#define NET_EVENTLOOP_H_

#include <stdint.h>
#include <sys/time.h>
#include <tr1/memory>
#include <tr1/functional>
#include <vector>
#include <queue>

#include "noncopyable.h"

namespace net 
{

	typedef std::tr1::function<void ()> EventLoopUpdate;
	typedef std::tr1::function<void ()> TimerCallback;

	struct Timer 
	{
		time_t timer;
		TimerCallback cb;
	};

	struct TimerComparer 
	{
		bool operator()(const Timer& lhs, const Timer& rhs)
		{
			return lhs.timer > rhs.timer; 
		}
	};

	// In milliseconds
	time_t CalculateTimer(const timeval& tv);

	class EventLoop
	{
	public:
		static EventLoop& instance()
		{
			static EventLoop loop;
			return loop;
		}

		// EventLoopUpdate will be invoked every update_interval ms
  	// Mininal update_interval should >= kDefaultPollTimeoutMs
  	void set_update(EventLoopUpdate update, int update_interval);

  	// Will loops forever, must be the last function in main()
  	// Will be exit in the next run when call quit()
  	void loop();

  	// Loop() will exit in the next run
  	void quit();

  	void run_after(int time_ms, TimerCallback cb);
  private:
  	EventLoop();

  	bool _quit;
  	EventLoopUpdate _update;
  	int _update_interval;

  	typedef std::priority_queue<Timer, std::vector<Timer>, TimerComparer>
          TimerQueue;
  	TimerQueue _timer_queue;

  	DISALLOW_COPY_AND_ASSIGN(EventLoop);
	};

}

#endif