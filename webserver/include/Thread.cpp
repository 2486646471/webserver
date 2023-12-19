#include"Thread.h"
#include"CurrentThread.h"
#include <sys/prctl.h>
#include <sys/types.h>
#include <assert.h>
#include <string>
#include <stdint.h>
#include <sys/syscall.h>
using namespace std;

namespace CurrentThread {
	__thread int m_cache_tid = 0;
	__thread char m_tid_string[32];
	__thread int m_tid_length = 6;
	__thread const char* m_thread_name = "default";
}
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

//缓存当前的线程id
void CurrentThread::cacheTid() {
	if (m_cache_tid == 0) {
		m_cache_tid = gettid();
    		m_tid_length = snprintf(m_tid_string, sizeof(m_tid_string), "%5d ", m_cache_tid);
	}
}

struct ThreadData {
	typedef Thread::ThreadFunc ThreadFunc;
	ThreadFunc m_func;
	string m_name;
	pid_t* m_tid;
	CountDownLatch* m_latch;

  	ThreadData(const ThreadFunc& func, const string& name, pid_t* tid,
             CountDownLatch* latch): 
		m_func(func), m_name(name), m_tid(tid), m_latch(latch) {}

	void runInThread() {
		//缓存线程id
    		*m_tid = CurrentThread::tid();
		m_tid = NULL;

		//通知调用它的程序
    		m_latch -> countDown();
    		m_latch = NULL;

    		CurrentThread::m_thread_name = m_name.empty() ? "Thread" : m_name.c_str();
    		prctl(PR_SET_NAME, CurrentThread::m_thread_name);
    		m_func();
    		CurrentThread::m_thread_name = "finished";
  	}
};

Thread::Thread(const ThreadFunc& func, const string& n):
	m_started(false), 
	m_joined(false), 
	m_pthread_id(0),
	m_tid(0),
      	m_func(func),
	m_name(n),
        m_latch(1) {
  	setDefaultName();
}

Thread::~Thread() {
	if (m_started && !m_joined) pthread_detach(m_pthread_id);
}

void Thread::setDefaultName() {
	if (m_name.empty()) {
    		m_name = "Thread";
	}
}

void* startThread(void* obj) {
	ThreadData* data = static_cast<ThreadData*>(obj);
  	data -> runInThread();
  	delete data;
  	return NULL;
}


void Thread::start() {
	assert(!m_started);
	m_started = true;
	ThreadData* data = new ThreadData(m_func, m_name, &m_tid, &m_latch);
  	if (pthread_create(&m_pthread_id, NULL, &startThread, data)) {
    		m_started = false;
    		delete data;
	} 
	else {
    		m_latch.wait();
    		assert(m_tid > 0);
  	}
}

int Thread::join() {
	assert(m_started);
	assert(!m_joined);
	m_joined = true;
  	return pthread_join(m_pthread_id, NULL);
}
