#include "EventLoopThread.h"

EventLoopThread::EventLoopThread()
    : m_loop(NULL),
      m_exiting(false),
      m_thread(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
      m_mutex(),
      m_cond(m_mutex) {}


EventLoopThread::~EventLoopThread() {
	m_exiting = true;
  	if (m_loop != NULL) {
    		m_loop -> quit();
    		m_thread.join();
  	}
}


EventLoop* EventLoopThread::startLoop() {
	assert(!m_thread.started());
	//启动线程，执行threadFunc函数
	m_thread.start();

  	{
    		MutexLockGuard lock(m_mutex);
    		while (m_loop == NULL) cond_.wait();
  	}
  	return m_loop;
}



void EventLoopThread::threadFunc() {
	EventLoop loop;
	{
    		MutexLockGuard lock(m_mutex);
    		m_loop = &loop;
    		m_cond.notify();
  	}
  	loop.loop();
  	m_loop = NULL;
}
