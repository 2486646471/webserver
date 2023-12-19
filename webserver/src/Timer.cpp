#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>

TimerNode::TimerNode(std::shared_ptr<HttpData> request_data, int timeout)
	: m_deleted(false), m_httpp_data(request_data) {
	struct timeval now;
  	gettimeofday(&now, NULL);
  	// 以毫秒计
  	m_expired_time = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode() {
	if (m_http_data) m_http_data -> handleClose();
}

void TimerNode::update(int timeout) {
	struct timeval now;
	gettimeofday(&now, NULL);
	m_expired_time_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid() {
	struct timeval now;
  	gettimeofday(&now, NULL);
  	size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
  	if (temp < m_expired_time) return true;
  	else {
    		this -> setDeleted();
    		return false;
  	}
}



void TimerManager::addTimer(std::shared_ptr<HttpData> request_data, int timeout) {
	SPTimerNode new_node(new TimerNode(request_data, timeout));
  	m_timer_node_queue.push(new_node);
  	request_data -> linkTimer(new_node);
}

void TimerManager::handleExpiredEvent() {
	while (!m_timer_node_queue.empty()) {
    		SPTimerNode ptimer_now = timerNodeQueue.top();
    		if (ptimer_now -> isDeleted())
      			timerNodeQueue.pop();
    		else if (ptimer_now -> isValid() == false)
      			timerNodeQueue.pop();
    		else
     			 break;
  	}
}





