#include"Server.h"

Server::Server(EventLoop *loop, int thread_num, int port)
	: m_loop(loop),
	m_thread_num(thread_num),
	m_thread_pool(new EventLoopThreadPool(m_loop, thread_num)),
	m_started(false),
        m_accept_channel(new Channel(m_loop)),
      	m_port(port),

	//创建一个套接字文件
        m_listen_fd(socket_bind_listen(port)) {
  		m_accept_channel -> setFd(listenFd);
  		handle_for_sigpipe();
		//设置socketfd为非阻塞
  		if (setSocketNonBlocking(m_listen_fd) < 0) {
    			perror("set socket non block failed");
    			abort();
  		}
}


void Server::start() {
	//初始化线程池
	m_thread_pool -> start();

	//初始化m_accept_channel
  	m_accept_channel -> setEvents(EPOLLIN | EPOLLET);
  	m_accept_channel -> setReadHandler(bind(&Server::handNewConn, this));
  	m_accept_channel -> setConnHandler(bind(&Server::handThisConn, this));

	//将这个channel放入epoll
  	m_loop -> addToPoller(m_accept_channel, 0);
  	m_started = true;
}

//当监听socket得到EPOLLIN事件的时候，调用这个函数
void Server::handNewConn() {
	struct sockaddr_in client_addr;
  	memset(&client_addr, 0, sizeof(struct sockaddr_in));
  	socklen_t client_addr_len = sizeof(client_addr);

	//接受连接
  	int accept_fd = 0;
  	while ((accept_fd = accept(m_listen_fd, (struct sockaddr *)&client_addr, &client_addr_len)) > 0) {
		//将建立的连接下发到下面的线程来监听
    		EventLoop* loop = m_thread_pool -> getNextLoop();
    		LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"<< ntohs(client_addr.sin_port);
		
		// 限制服务器的最大并发连接数
    		if (accept_fd >= MAXFDS) {
      			close(accept_fd);
      			continue;
    		}
    		
		// 设为非阻塞模式
    		if (setSocketNonBlocking(accept_fd) < 0) {
      			LOG << "Set non block failed!";
      			// perror("Set non block failed!");
      			return;
    		}
    		setSocketNodelay(accept_fd);

		//创建一个HttpData
    		shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
		//将channel与HttpData绑定到一起
    		req_info -> getChannel()-> setHolder(req_info);
		//如果阻塞在epoll_wait,唤醒下发的线程
    		loop -> queueInLoop(std::bind(&HttpData::newEvent, req_info));
  	}

	//因为这个函数完成后，会将events设置为零，所以要再次设置
  	m_accept_channel -> setEvents(EPOLLIN | EPOLLET);
}
