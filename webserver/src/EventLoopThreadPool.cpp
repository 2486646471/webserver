#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, int num_threads)
	: m_loop(loop),	//这里的loop是main函数的loop，即主循环
       	m_started(false), 
	m_num_threads(num_threads), 
	m_next(0) {
		if (num_threads <= 0) {
    			LOG << "numThreads_ <= 0";
    			abort();
  		}
}

void EventLoopThreadPool::start() {
	m_loop -> assertInLoopThread();
  	m_started = true;

	//创建线程,并启动线程
  	for (int i = 0; i < m_num_threads; ++i) {
    		std::shared_ptr<EventLoopThread> t(new EventLoopThread());
    		m_threads.push_back(t);
    		m_loops.push_back( t -> startLoop());
  	}
}

EventLoop* EventLoopThreadPool::getNextLoop() {
	m_loop -> assertInLoopThread();
  	assert(m_started);
  	EventLoop* loop = m_loop;
  	if (!m_loops.empty()) {
    		loop = m_loops[m_next];
    		m_next = (m_next + 1) % m_num_threads;
  	}
  	return loop;
}

