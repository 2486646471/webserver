#include"EventLoop.h"


__thread EventLoop* m_loop_in_thread = 0;

int createEventfd() {
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  	if (evtfd < 0) {
    		LOG << "Failed in eventfd";
    		abort();
  	}
  	return evtfd;
}

EventLoop::EventLoop()
	: m_looping(false),
      	  m_poller(new Epoll()),
       	  m_wakeup_fd(createEventfd()),
      	  m_quit(false),
          m_event_handling(false),
          m_calling_pending_functors(false),
          m_thread_id(CurrentThread::tid()),
          m_wakeup_channel(new Channel(this, m_wakeup_fd)) {
		  m_loopInThisThread = this;
		  //将m_wakeup_fd加入epoll监听队列中
		  m_wakeup_channel -> setEvents(EPOLLIN | EPOLLET);
		  m_wakeup_channel -> setReadHandler(bind(&EventLoop::handleRead, this));
		  m_wakeup_channel_-> setConnHandler(bind(&EventLoop::handleConn, this));
		  m_poller -> epoll_add(m_wakeup_channel, 0);
}

void EventLoop::handleRead() {
	uint64_t one = 1;
  	ssize_t n = readn(m_wakeup_fd, &one, sizeof one);
  	if (n != sizeof one) {
    		LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  	}
  	m_wakeup_channel -> setEvents(EPOLLIN | EPOLLET);
}
void EventLoop::handleConn() {
	updatePoller(m_wakeup_channel, 0);
}

//唤醒就是向wake_fd中写入
void EventLoop::wakeup() {
	uint64_t one = 1;
  	ssize_t n = writen(m_wakeup_fd, (char*)(&one), sizeof one);
  	if (n != sizeof one) {
    		LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  	}
}

//放入的函数是将channel加入epoll监听队列中
void EventLoop::queueInLoop(Functor&& cb) {
	{
    		MutexLockGuard lock(m_mutex);
    		m_pending_functors.emplace_back(std::move(cb));
  	}
  	if (!isInLoopThread() || m_calling_pending_functors) wakeup();
}


void EventLoop::loop() {
	assert(!m_looping);
  	assert(isInLoopThread());
  	m_looping = true;
  	m_quit = false;
  	std::vector<SP_Channel> ret;
  	while (!quit_) {
		ret.clear();
    		ret = m_poller -> poll();
    		m_event_handling = true;
    		for (auto& it : ret) it -> handleEvents();
    		m_eventHandling = false;
    		doPendingFunctors();
    		m_poller -> handleExpired();
  	}
  	m_looping = false;
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
  	m_calling_pending_functors = true;
	{
		MutexLockGuard lock(m_mutex);
		functors.swap(m_pending_functors);
  	}
	for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  	m_calling_pending_functors = false;
}

void EventLoop::quit() {
	m_quit = true;
  	if (!isInLoopThread()) {
    		wakeup();
  	}
}

