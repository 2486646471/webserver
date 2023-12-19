#pragma once
#include<pthread.h>
#include<errno.h>
#include<time.h>
#include"noncopyable.h"
#include"MutexLock.h"

class Condition : noncopyable {
private:
	MutexLock& m_mutex;
	pthread_cond_t m_cond;
public:
	explicit Condition(MutexLock& mutex) : m_mutex(mutex) {
		pthread_cond_init(&m_cond, NULL);
	}
	
	void wait() {
		pthread_cond_wait(&m_cond, m_mutex.get());
	}
	void notify() {
		pthread_cond_signal(&m_cond);
	}
	void notifyAll() {
		pthread_cond_broadcast(&m_cond);
	}
	
	bool waitForSeconds(int seconds) {
		struct timespec reltime;
    		clock_gettime(CLOCK_REALTIME, &reltime);
    		reltime.tv_sec += static_cast<time_t>(seconds);
    		return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.get(), &reltime);
	}

	~Condition() {
		pthread_cond_destroy(&m_cond);
	}
};

