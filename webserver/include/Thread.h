#pragma once
#include"noncopyable.h"
#include"CountDownLatch.h"
#include<string>
#include<pthread.h>
#include <memory>
#include<unistd.h>
#include<functional>

class Thread {
public:
	typedef std::function<void()> ThreadFunc;
	explicit Thread(const ThreadFunc&, const std::string& name = std::string());
	~Thread();
  	void start();
  	int join();
  	bool started() const { return m_started; }
  	pid_t tid() const { return m_tid; }
  	const std::string& name() const { return m_name; }
private:
	void setDefaultName();
	bool m_started;
	bool m_joined;
	pthread_t m_pthread_id;
	pid_t m_tid;
	ThreadFunc m_func;
	std::string m_name;
	CountDownLatch m_latch;
};
