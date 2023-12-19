#include"Util.h"
#include<unistd.h>
#include<fcntl.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

const int MAX_BUFF = 4096;

ssize_t readn(int fd, void *buff, size_t n) {
	ssize_t nread = 0;
	ssize_t read_sum = 0;
	char* ptr = (char *)buff;

	while (n > 0) {
		if ((nread = read(fd, ptr, n)) < 0) {
			if (errno == EINTR) continue;
      			else if (errno == EAGAIN) return read_sum; 
			else return -1;
    		}
		else if (nread == 0) break;
    		read_sum += nread;
    		n -= nread;
    		ptr += nread;
  	}
	return read_sum;
}


ssize_t readn(int fd, std::string &in_buffer, bool &zero) {
	ssize_t nread = 0;
	ssize_t read_sum = 0;
	while (true) {
    		char buff[MAX_BUFF];
		if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
      			if (errno == EINTR) continue;
      			else if (errno == EAGAIN) return read_sum; 
			else {
        			perror("read error");
        			return -1;
      			}
    		} 
		else if (nread == 0) {
      			zero = true;
      			break;
    		}
    		read_sum += nread;
    		in_buffer += std::string(buff, buff + nread);
  	}
  	return read_sum;
}

ssize_t readn(int fd, std::string &in_buffer) {
	ssize_t nread = 0;
  	ssize_t read_sum = 0;
  	while (true) {
    		char buff[MAX_BUFF];
    		if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
      			if (errno == EINTR) continue;
      			else if (errno == EAGAIN) return read_sum; 
			else {
        			perror("read error");
        			return -1;
      			}
    		} 
		else if (nread == 0) {
      			break;
    		}
    		read_sum += nread;
    		in_buffer += std::string(buff, buff + nread);
  	}
  	return read_sum;
}

ssize_t writen(int fd, void *buff, size_t n) {
  	ssize_t nwritten = 0;
  	ssize_t write_sum = 0;
  	char *ptr = (char *)buff;
  	while (n > 0) {
    		if ((nwritten = write(fd, ptr, n)) <= 0) {
      			if (nwritten < 0) {
        			if (errno == EINTR) {
          				continue;
        			} 
				else if (errno == EAGAIN) {
          				return write_sum;
        			} 
				else return -1;
      			}
    		}
    		write_sum += nwritten;
    		n -= nwritten;
    		ptr += nwritten;
  	}
  	return write_sum;
}

ssize_t writen(int fd, std::string &sbuff) {
	size_t n = sbuff.size();
        ssize_t nwritten = 0;
        ssize_t write_sum = 0;
  	const char *ptr = sbuff.c_str();
  	while (n > 0) {
    		if ((nwritten = write(fd, ptr, n)) <= 0) {
      			if (nwritten < 0) {
        			if (errno == EINTR) {
          				continue;
        			} 
				else if (errno == EAGAIN) break;
        			else return -1;
    			}
		}
    		write_sum += nwritten;
    		n -= nwritten;
    		ptr += nwritten;
  	}
  	if (write_sum == static_cast<int>(sbuff.size()))
    		sbuff.clear();
  	else
    		sbuff = sbuff.substr(write_sum);
  	return write_sum;
}


void handleForSigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
  	sigaction(SIGPIPE, &sa, NULL);
}


int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
  	if (flag == -1) return -1;
  	flag |= O_NONBLOCK;
  	if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  	return 0;
}

void setSocketNodelay(int fd) {
	int enable = 1;
  	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}


void setSocketNoLinger(int fd) {
	struct linger linger_;
  	linger_.l_onoff = 1;
  	linger_.l_linger = 30;
  	setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
}


void shutDownWR(int fd) {
	shutdown(fd, SHUT_WR);
}

int socket_bind_listen(int port) {
	// 检查port值，取正确区间范围
  	if (port < 0 || port > 65535) return -1;

  	// 创建socket(IPv4 + TCP)，返回监听描述符
  	int listen_fd = 0;
  	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

  	// 端口复用
  	int optval = 1;
  	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		close(listen_fd);
    		return -1;
  	}

  	// 设置服务器IP和Port，和监听描述副绑定
  	struct sockaddr_in server_addr;
  	bzero((char *)&server_addr, sizeof(server_addr));
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	server_addr.sin_port = htons((unsigned short)port);
  	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1) {
    		close(listen_fd);
    		return -1;
  	}

  	// 开始监听，最大等待队列长为LISTENQ
  	if (listen(listen_fd, 2048) == -1) {
    		close(listen_fd);
    		return -1;
  	}

  	// 无效监听描述符
  	if (listen_fd == -1) {
    		close(listen_fd);
    		return -1;
  	}
  	return listen_fd;
}
