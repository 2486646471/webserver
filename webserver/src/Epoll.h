#pragma once
#include <vector>
#include "Channel.h"

class Epoll {
private:
 	static const int MAXFDS = 100000;
 	int m_epoll_fd;

	//记录发生的事件
 	std::vector<epoll_event> m_events;

	//所监听的channel
 	std::shared_ptr<Channel> m_channel[MAXFDS];
 	std::shared_ptr<HttpData> m_http[MAXFDS];
 	TimerManager m_timer_manager;

public:
	Epoll();
  	~Epoll();
  	void epollAdd(SP_Channel request, int timeout);
  	void epollMod(SP_Channel request, int timeout);
  	void epollDel(SP_Channel request);

  	std::vector<std::shared_ptr<Channel>> Poll();
  	std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  	void addTimer(std::shared_ptr<Channel> request_data, int timeout);
  	int getEpollFd() { return m_epoll_fd; }
  	void handleExpired();
};
