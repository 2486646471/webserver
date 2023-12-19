#pragma once
#include <functional>
#include <string>
#include <vector>
#include "CountDownLatch.h"
#include "LogStream.h"
#include "MutexLock.h"
#include "Thread.h"
#include "noncopyable.h"


class AsyncLogging : noncopyable {
public:
	AsyncLogging(const std::string file_name, int flush_interval = 2);
	~AsyncLogging() {
		if (m_running) stop();
	}

	void append(const char* logline, int len);

	void start() {
		m_running = true;

		//启动异步线程，请确保它启动再做其它
		m_thread.start();
		m_latch.wait();
	}

	void stop() {
		m_running = false;
  		m_cond.notify();
  		m_thread.join();
	}
private:
	void threadFunc();
	typedef FixedBuffer<big_buffer> Buffer;
  	typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  	typedef std::shared_ptr<Buffer> BufferPtr; 
	const int m_flush_interval;
	bool m_running;
	std::string m_name;
	Thread m_thread;
	MutexLock m_mutex;
	Condition m_cond;
	BufferPtr current_buffer;
 	BufferPtr next_buffer;
  	BufferVector buffers;
  	CountDownLatch m_latch;
};
