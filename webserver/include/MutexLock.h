#pragma once
#include<pthread.h>
#include"noncopyable.h"

class MutexLock:noncopyable {
private:
	pthread_mutex_t m_mutex;
	friend class Condition;
public:
	MutexLock() {
		pthread_mutex_init(&m_mutex, NULL);
	}
	~MutexLock() {
		pthread_mutex_lock(&m_mutex);
		pthread_mutex_destroy(&m_mutex);
	}
	
	void lock() {
		pthread_mutex_lock(&m_mutex);
	}
	void unlock() {
		pthread_mutex_unlock(&m_mutex);
	}
	pthread_mutex_t* get() {
		return &m_mutex;
	}
};


class MutexLockGuard:noncopyable {
private:
	MutexLock& m_mutex;
public:	
	explicit MutexLockGuard(MutexLock& mutex): m_mutex(mutex) {
		m_mutex.lock();
	}
	~MutexLockGuard() {
		m_mutex.unlock();
	}
};

