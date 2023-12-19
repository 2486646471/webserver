#pragma once
#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "base/Logging.h"
#include "base/noncopyable.h"


class EventLoopThreadPool : noncopyable {
public:
	EventLoopThreadPool(EventLoop* loop, int num_threads);

  	~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }
  	void start();
  	EventLoop* getNextLoop();

private:
  	EventLoop* m_loop;
  	bool m_started;
  	int m_num_threads;
  	int m_next;
  	std::vector<std::shared_ptr<EventLoopThread>> m_threads;
  	std::vector<EventLoop*> m_loops;
};
