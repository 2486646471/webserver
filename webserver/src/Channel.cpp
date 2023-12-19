#include"Channel.h"

Channel::Channel(EventLoop* loop)
    : m_loop(loop), m_events(0), m_lastEvents(0), m_fd(0) {}

Channel::Channel(EventLoop *loop, int fd)
    : m_loop(loop), m_fd(fd), m_events(0), m_lastEvents(0) {}

Channel::~Channel() {}

int Channel::getFd() { return m_fd; }
void Channel::setFd(int fd) { m_fd = fd; }

void Channel::handleRead() {
	if (m_readHandler) {
    		m_readHandler();
  	}
}

void Channel::handleWrite() {
	if (m_writeHandler) {
    		m_writeHandler();
  	}
}

void Channel::handleConn() {
	if (m_connHandler) {
    		m_connHandler();
  	}
}
