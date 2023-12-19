#pragma once
#include "EventLoop.h"

class EventLoopThread : noncopyable {
public:
	EventLoopThread();
  	~EventLoopThread();
  	EventLoop* startLoop();

private:
	void threadFunc();
  	EventLoop* m_loop;
  	bool m_exiting;
  	Thread m_thread;
  	MutexLock m_mutex;
  	Condition m_cond;
};
