#include "ThreadPool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;

int ThreadPool::m_thread_count = 0;
int ThreadPool::m_queue_size = 0;
int ThreadPool::m_head = 0;
int ThreadPool::m_tail = 0;
int ThreadPook::m_count = 0;
int ThreadPool::m_shutdown = 0;
int ThreadPool::m_started = 0;


//创建线程池
int ThreadPool::threadpoolCreate(int thread_count, int queue_size){
	bool err = false;
    	do{
        	if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
            		thread_count = 4;
            		queue_size = 1024;
        	}

        	m_thread_count = 0;
        	m_queue_size = queue_size;
        	head = tail = 0;
        	shutdown = started = 0;

        	m_threads.resize(thread_count);
        	queue.resize(queue_size);

        	for(int i = 0; i < thread_count; ++i) {
			if(pthread_create(&m_threads[i], NULL, threadpool_thread, (void*)(0)) != 0) {
                		return -1;
            		}
            		m_thread_count++;
            		m_started++;
        	}
    	} 
	while(false);

    	if (err){
        	return -1;
    	}
    	return 0;
}

int ThreadPool::threadpoolAdd(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun) {
	int next, err = 0;
    	if(pthread_mutex_lock(&lock) != 0)
        	return THREADPOOL_LOCK_FAILURE;
    	do {
        	next = (tail + 1) % queue_size;
        	// 队列满
        	if(m_count == queue_size) {
            		err = THREADPOOL_QUEUE_FULL;
            		break;
        	}
        	// 已关闭
        	if(shutdown) {
            		err = THREADPOOL_SHUTDOWN;
           	 	break;
        	}
        	queue[tail].fun = fun;
        	queue[tail].args = args;
        	tail = next;
        	m_count++;
        
        	if(pthread_cond_signal(&notify) != 0) {
            		err = THREADPOOL_LOCK_FAILURE;
            		break;
        	}
    	} while(false);

    	if(pthread_mutex_unlock(&lock) != 0)
        	err = THREADPOOL_LOCK_FAILURE;
    	return err;
}


void* ThreadPool::threadpoolThread(void *args) {
	while (true) {
        	ThreadPoolTask task;
        	pthread_mutex_lock(&m_lock);
        	while((m_count == 0) && (!m_shutdown)) {
        		pthread_cond_wait(&notify, &lock);
        	}
        	if((m_shutdown == immediate_shutdown) ||((m_shutdown == graceful_shutdown) && (m_count == 0))) {
            		break;
        	}
        	task.fun = m_queue[head].fun;
        	task.args = m_queue[head].args;
        	m_queue[head].fun = NULL;
        	m_queue[head].args.reset();
        	m_head = (m_head + 1) % m_queue_size;
        	m_count--;
        	pthread_mutex_unlock(&m_lock);
        	(task.fun)(task.args);
    	}
	pthread_mutex_lock(&m_lock);
    	m_started--;
    	pthread_mutex_unlock(&lock);
    	pthread_exit(NULL);
    	return(NULL);
}
