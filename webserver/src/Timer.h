#pragma once
#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include "HttpData.h"

class TimerNode {
public:
	TimerNode(std::shared_ptr<HttpData> request_data, int timeout);
  	~TimerNode();
  	TimerNode(TimerNode &tn);
  	void update(int timeout);
  	bool isValid();
  	void clearReq();
  	void setDeleted() { m_deleted = true; }
  	bool isDeleted() const { return m_deleted; }
  	size_t getExpTime() const { return m_expired_time; }

private:
  	bool m_deleted;
  	size_t m_expired_time;
  	std::shared_ptr<HttpData> m_http_data;
};

struct TimerCmp {
	bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const {
    		return a->getExpTime() > b->getExpTime();
  	}
};

class TimerManager {
public:
	TimerManager();
  	~TimerManager();
  	void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
  	void handleExpiredEvent();

private:
  	typedef std::shared_ptr<TimerNode> SPTimerNode;
  	std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> m_timer_node_queue;
};
