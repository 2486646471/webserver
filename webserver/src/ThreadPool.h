#pragma once
#include "Channel.h"
#include <pthread.h>
#include <vector>

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

//线程池的最大线程数和最多任务数
const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

struct ThreadPoolTask{
	std::function<void(std::shared_ptr<void>)> fun;
    	std::shared_ptr<void> args;
};

class ThreadPool{
private:
    static pthread_mutex_t m_lock;
    static pthread_cond_t m_notify;

    static std::vector<pthread_t> m_threads;
    static std::vector<ThreadPoolTask> m_queue;
    static int m_thread_count;
    static int m_queue_size;	//任务队列大小
    static int m_head;
    static int m_tail;
    static int m_count;	//任务数量
    static int m_shutdown;
    static int m_started;	//工作的线程数
public:
    static int threadpoolCreate(int thread_count, int queue_size);
    static int threadpoolAdd(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun);
    static int threadpoolDestroy(ShutDownOption shutdown_option = graceful_shutdown);
    static int threadpoolFree();
    static void *threadpoolThread(void *args);
};

