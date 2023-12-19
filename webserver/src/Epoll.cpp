#include "Epoll.h"

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;
typedef shared_ptr<Channel> SP_Channel;

Epoll::Epoll() : m_epoll_fd(epoll_create(EPOLL_CLOEXEC)), m_events(EVENTSNUM) {
	assert(m_epoll_fd > 0);
}
Epoll::~Epoll() {}

// 注册新描述符
void Epoll::epollAdd(SP_Channel request, int timeout) {
	int fd = request -> getFd();
  	if (timeout > 0) {
    		addTimer(request, timeout);
    		fd2http_[fd] = request->getHolder();
  	}

  	struct epoll_event event;
  	event.data.fd = fd;
  	event.events = request -> getEvents();
  	request -> EqualAndUpdateLastEvents();
  	m_channel[fd] = request;

  	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
    		perror("epoll_add error");
    		m_channel[fd].reset();
  	}
}

// 修改描述符状态
void Epoll::epollMod(SP_Channel request, int timeout) {
	if (timeout > 0) addTimer(request, timeout);
  	int fd = request -> getFd();

	//如果想要监听的事件和上一次记录的事件相同
        if (!request -> EqualAndUpdateLastEvents()) {
    		struct epoll_event event;
    		event.data.fd = fd;
    		event.events = request -> getEvents();
    		if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
      			perror("epoll_mod error");
      			m_channel[fd].reset();
    		}
  	}
}

// 从epoll中删除描述符
void Epoll::epollDel(SP_Channel request) {
	int fd = request -> getFd();
  	struct epoll_event event;
  	event.data.fd = fd;
  	event.events = request -> getLastEvents();
  	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0) {
    		perror("epoll_del error");
  	}
  	m_channel[fd].reset();
}


// 返回活跃事件
std::vector<SP_Channel> Epoll::Poll() {
	while (true) {
    		int event_count = epoll_wait(m_epoll_fd, &*m_events.begin(), m_events.size(), EPOLLWAIT_TIME);
    		if (event_count < 0) perror("epoll wait error");
    		std::vector<SP_Channel> req_data = getEventsRequest(event_count);
    		if (req_data.size() > 0) return req_data;
  	}
}
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num) {
	std::vector<SP_Channel> req_data;
  	for (int i = 0; i < events_num; ++i) {
    		// 获取有事件产生的描述符
    		int fd = m_events[i].data.fd;
    		SP_Channel cur_req = m_channel[fd];
    		if (cur_req) {
      			cur_req -> setRevents(m_events[i].events);
      			cur_req -> setEvents(0);
      			req_data.push_back(cur_req);
    		} 
		else {
      			LOG << "SP cur_req is invalid";
    		}
  	}
	return req_data;
}

