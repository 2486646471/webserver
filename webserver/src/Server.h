#pragma once


class Server {
public:
  	Server(EventLoop *loop, int thread_num, int port);
	~Server() {}
	EventLoop *getLoop() const { return m_loop; }
	void start();
	void handNewConn();
	void handThisConn() { m_loop -> updatePoller(m_accept_channel); }

private:
	EventLoop *m_loop;
	int m_thread_num;
  	std::unique_ptr<EventLoopThreadPool> m_thread_pool;
  	bool m_started;
  	std::shared_ptr<Channel> m_accept_channel;
  	int m_port;
  	int m_listen_fd;
  	static const int MAXFDS = 100000;
};
