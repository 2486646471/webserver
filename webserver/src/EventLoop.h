#pragma once

class EventLoop {
public:
	typedef std::function<void()> Functor;
  	EventLoop();
  	~EventLoop();
  	void loop();
  	void quit();
  	void runInLoop(Functor&& cb);
  	void queueInLoop(Functor&& cb);
  	bool isInLoopThread() const { return m_thread_id == CurrentThread::tid(); }
  	void assertInLoopThread() { assert(isInLoopThread()); }
  	void shutdown(shared_ptr<Channel> channel) {shutDownWR(channel -> getFd());}
 

	void removeFromPoller(shared_ptr<Channel> channel) {
    		m_poller -> epoll_del(channel);
  	}
  	void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {
    		m_poller -> epoll_mod(channel, timeout);
  	}
  	void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {
    		m_poller -> epoll_add(channel, timeout);
  	}

private:
	// 声明顺序 wakeupFd_ > pwakeupChannel_
 	bool m_looping;
 	shared_ptr<Epoll> m_poller;
 	int m_wakeup_fd;
 	bool m_quit;
 	bool m_event_handling;
 	mutable MutexLock m_mutex;

 	std::vector<Functor> m_pending_functors;
 	bool m_calling_pending_functors;
 	const pid_t m_thread_id;
 	shared_ptr<Channel> m_wakeup_channel;

 	void wakeup();
 	void handleRead();
 	void doPendingFunctors();
 	void handleConn();
};
