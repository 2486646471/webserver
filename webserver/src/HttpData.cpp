#include "HttpData.h"

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;

//连接再服务器上，但未收到新请求的最长时间
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
//默认的保持连接时间
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;
void MimeType::init() {
	mime[".html"] = "text/html";
  	mime[".avi"] = "video/x-msvideo";
  	mime[".bmp"] = "image/bmp";
  	mime[".c"] = "text/plain";
  	mime[".doc"] = "application/msword";
  	mime[".gif"] = "image/gif";
  	mime[".gz"] = "application/x-gzip";
  	mime[".htm"] = "text/html";
  	mime[".ico"] = "image/x-icon";
  	mime[".jpg"] = "image/jpeg";
  	mime[".png"] = "image/png";
  	mime[".txt"] = "text/plain";
  	mime[".mp3"] = "audio/mp3";
  	mime["default"] = "text/html";
}


std::string MimeType::getMime(const std::string &suffix) {
	pthread_once(&once_control, MimeType::init);
  	if (mime.find(suffix) == mime.end()) return mime["default"];
  	else return mime[suffix];
}


HttpData::HttpData(EventLoop* loop, int connfd)
	: m_loop(loop),
      	m_channel(new Channel(loop, connfd)),
      	m_fd(connfd),
      	m_error(false),
      	m_connection_state(H_CONNECTED),
      	m_method(METHOD_GET),
      	m_http_version(HTTP_11),
      	m_read_pos(0),
      	m_state(STATE_PARSE_URI),
      	m_hstate(H_START),
      	m_keep_alive(false) {
  	m_channel -> setReadHandler(bind(&HttpData::handleRead, this));
  	m_channel -> setWriteHandler(bind(&HttpData::handleWrite, this));
  	m_channel -> setConnHandler(bind(&HttpData::handleConn, this));
}


void HttpData::reset() {
  	m_file_name.clear();
  	m_path.clear();
  	m_read_pos = 0;
  	m_state = STATE_PARSE_URI;
  	m_hstate = H_START;
  	m_headers.clear();

  	if (m_timer.lock()) {
    		shared_ptr<TimerNode> my_timer(timer_.lock());
    		my_timer -> clearReq();
    		timer_.reset();
  	}
}

void HttpData::seperateTimer() {
	if (m_timer.lock()) {
    		shared_ptr<TimerNode> my_timer(m_timer.lock());
    		my_timer->clearReq();
    		timer_.reset();
  	}
}

void HttpData::handleRead() {
	__uint32_t &events = m_channel -> getEvents();
  	do {
    		bool zero = false;
    		int read_num = readn(m_fd, m_in_buffer, zero);
    		LOG << "Request: " << m_in_buffer;

		//如果连接状态为正在断开连接
    		if (m_connection_state == H_DISCONNECTING) {
      			m_in_buffer.clear();
      			break;
    		};

    		if (read_num < 0) {
      			perror("1");
      			m_error = true;
      			handleError(m_fd, 400, "Bad Request");
      			break;
    		}

    		else if (zero) {
      			// 有请求出现但是读不到数据
      			m_connection_state = H_DISCONNECTING;
      			if (read_num == 0) {
        			break;
      			}
    		}
		

		//判断各种状态来解析URI
    		if (m_state == STATE_PARSE_URI) {
			URIState flag = this -> parseURI();
      			if (flag == PARSE_URI_AGAIN) break;
      			else if (flag == PARSE_URI_ERROR) {
        			perror("2");
        			LOG << "FD = " << m_fd << "," << m_in_buffer << "******";
        			m_in_buffer.clear();
        			m_error = true;
        			handleError(m_fd, 400, "Bad Request");
        			break;
      			}
			else m_state = STATE_PARSE_HEADERS;
    		}

		
		//解析头部
    		if (m_state == STATE_PARSE_HEADERS) {
      			HeaderState flag = this -> parseHeaders();
      			if (flag == PARSE_HEADER_AGAIN) break;
      			else if (flag == PARSE_HEADER_ERROR) {
        			perror("3");
        			m_error = true;
        			handleError(m_fd, 400, "Bad Request");
        			break;
      			}

      			if (m_method == METHOD_POST) {
        			m_state = STATE_RECV_BODY;
      			} 
			else {
        			m_state = STATE_ANALYSIS;
      			}
    		}
		
		
    		if (state_ == STATE_RECV_BODY) {
      			int content_length = -1;
      			if (m_headers.find("Content-length") != m_headers.end()) {
        			content_length = stoi(m_headers["Content-length"]);
      			} 
			else {
        			m_error = true;
        			handleError(m_fd, 400, "Bad Request: Lack of argument (Content-length)");
        			break;
      			}
      			if (static_cast<int>(m_in_buffer.size()) < content_length) break;
      			m_state = STATE_ANALYSIS;
    		}
    		

		if (m_state == STATE_ANALYSIS) {
      			AnalysisState flag = this -> analysisRequest();
      			if (flag == ANALYSIS_SUCCESS) {
        			m_state = STATE_FINISH;
        			break;
      			} 
			else {
        			m_error = true;
        			break;
      			}
    		}
  	} while (false);

  	if (!m_error) {
    		if (m_outBuffer.size() > 0) {
      			handleWrite();
    		}
		

    		if (!m_error && m_state == STATE_FINISH) {
      			this -> reset();
      			if (m_inBuffer.size() > 0) {
        			if (m_connection_state != H_DISCONNECTING) handleRead();
      			}
    		} 

		//继续监听读事件
		else if (!m_error && m_connection_state != H_DISCONNECTED) m_events |= EPOLLIN;
  	}
}


void HttpData::handleWrite() {
	if (!m_error && m_connection_state != H_DISCONNECTED) {
		__uint32_t &m_events = m_channel -> getEvents();
    		if (writen(m_fd, m_out_buffer) < 0) {
      			perror("writen");
      			m_events = 0;
      			m_error = true;
    		}

		//如果out_buffer还有数据
    		if (m_out_buffer.size() > 0) m_events |= EPOLLOUT;
  	}
}


void HttpData::handleConn() {
	seperateTimer();
  	__uint32_t &m_events = m_channel -> getEvents();

  	if (!m_error && m_connection_state == H_CONNECTED) {
    		if (m_events != 0) {
      			int timeout = DEFAULT_EXPIRED_TIME;
      			if (m_keep_alive) timeout = DEFAULT_KEEP_ALIVE_TIME;
      			if ((m_events & EPOLLIN) && (m_events & EPOLLOUT)) {
        			m_events = __uint32_t(0);
        			m_events |= EPOLLOUT;
      			}

      			m_events |= EPOLLET;
      			m_loop -> updatePoller(m_channel, timeout);

    		} 
		else if (m_keep_alive) {
      			m_events |= (EPOLLIN | EPOLLET);
      			int timeout = DEFAULT_KEEP_ALIVE_TIME;
			m_loop -> updatePoller(m_channel, timeout);
    		} 
		else {
      			m_events |= (EPOLLIN | EPOLLET);
      			int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
      			m_loop -> updatePoller(m_channel, timeout);
    		}
	} 

	else if (!m_error && connectionState_ == H_DISCONNECTING && (m_events & EPOLLOUT)) {
    		events_ = (EPOLLOUT | EPOLLET);
  	}

	else {
    		m_loop -> runInLoop(bind(&HttpData::handleClose, shared_from_this()));
  	}
}

void HttpData::handleError(int fd, int err_num, string short_msg) {
	short_msg = " " + short_msg;
	char send_buff[4096];
  	string body_buff, header_buff;
  	body_buff += "<html><title>哎~出错了</title>";
  	body_buff += "<body bgcolor=\"ffffff\">";
  	body_buff += to_string(err_num) + short_msg;
  	body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";

  	header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
  	header_buff += "Content-Type: text/html\r\n";
  	header_buff += "Connection: Close\r\n";
  	header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
  	header_buff += "Server: LinYa's Web Server\r\n";
  	header_buff += "\r\n";

  	// 错误处理不考虑writen不完的情况
  	sprintf(send_buff, "%s", header_buff.c_str());
  	writen(fd, send_buff, strlen(send_buff));
  	sprintf(send_buff, "%s", body_buff.c_str());
  	writen(fd, send_buff, strlen(send_buff));
}

void HttpData::handleClose() {
	m_connection_state = H_DISCONNECTED;
  	shared_ptr<HttpData> guard(shared_from_this());
  	m_loop -> removeFromPoller(m_channel);
}

URIState HttpData::parseURI() {
	string &str = m_in_buffer;
  	// 读到完整的请求行再开始解析请求
  	size_t pos = str.find('\r', m_read_pos);
  	if (pos < 0) {
    		return PARSE_URI_AGAIN;
  	}
	
  	// 去掉请求行所占的空间，节省空间
  	string request_line = str.substr(0, pos);
      	if(str.size() > pos + 1)
    		str = str.substr(pos + 1);
  	else
    		str.clear();

  	//判断是什么method
  	int pos_get = request_line.find("GET");
  	int pos_post = request_line.find("POST");
  	int pos_head = request_line.find("HEAD");
  	if (pos_get >= 0) {
    		pos = pos_get;
    		m_method = METHOD_GET;
  	} 
	else if (pos_post >= 0) {
    		pos = pos_post;
    		m_method = METHOD_POST;
  	} 
	else if (pos_head >= 0) {
    		pos = pos_head;
    		m_method = METHOD_HEAD;
  	} 
	else {
    		return PARSE_URI_ERROR;
  	}

  	//filename
  	pos = request_line.find("/", pos);
    	size_t _pos = request_line.find(' ', pos);
    	if (_pos < 0) return PARSE_URI_ERROR;
    	else {
      		if (_pos - pos > 1) {
        		m_file_name = request_line.substr(pos + 1, _pos - pos - 1);
			//如果有查询参数
        		size_t __pos = m_file_name.find('?');
        		if (__pos >= 0) {
          			m_file_name = m_file_name.substr(0, __pos);
        		}
      		}
      		else m_file_name = "index.html";
    	}
    	pos = _pos;


	//判断http版本
  	pos = request_line.find("/", pos);
  	if (pos < 0) return PARSE_URI_ERROR;
  	else {
    		if (request_line.size() - pos <= 3) return PARSE_URI_ERROR;
    		else {
      			string ver = request_line.substr(pos + 1, 3);
      			if (ver == "1.0") m_http_version = HTTP_10;
      			else if (ver == "1.1") m_http_version_ = HTTP_11;
      			else return PARSE_URI_ERROR;
    		}
  	}
  	return PARSE_URI_SUCCESS;
}


//解析头部
HeaderState HttpData::parseHeaders() {
	string &str = m_in_buffer;
  	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
  	int now_read_line_begin = 0;
  	bool not_finish = true;
  	size_t i = 0;
  	for (; i < str.size() && not_finish; ++i) {
    		switch (m_hstate) {
      			case H_START: {
        			if (str[i] == '\n' || str[i] == '\r') break;
        			m_hstate = H_KEY;
        			key_start = i;
        			now_read_line_begin = i;
        			break;
      			}
      			case H_KEY: {
        			if (str[i] == ':') {
          				key_end = i;
          				if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
          				m_hstate = H_COLON;
        			} 
				else if (str[i] == '\n' || str[i] == '\r') return PARSE_HEADER_ERROR;
        			break;
      			}
      			case H_COLON: {
        			if (str[i] == ' ') {
          				m_hstate = H_SPACES_AFTER_COLON;
        			} 
				else return PARSE_HEADER_ERROR;
        			break;
      			}
      			case H_SPACES_AFTER_COLON: {
        			m_hstate = H_VALUE;
        			value_start = i;
        			break;
      			}
      			case H_VALUE: {
        		 	if (str[i] == '\r') {
          				m_hstate = H_CR;
          				value_end = i;
          				if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
        			} 
				else if (i - value_start > 255) return PARSE_HEADER_ERROR;
        			break;
      			}
      			case H_CR: {
        			if (str[i] == '\n') {
          				m_hstate = H_LF;
          				string key(str.begin() + key_start, str.begin() + key_end);
         	 			string value(str.begin() + value_start, str.begin() + value_end);
          				m_headers[key] = value;
          				now_read_line_begin = i;
        			} 
				else return PARSE_HEADER_ERROR;
        			break;
      			}
      			case H_LF: {
        			if (str[i] == '\r') {
          				m_hstate = H_END_CR;
        			} 
				else {
          				key_start = i;
          				m_hstate = H_KEY;
        			}
        			break;
      			}
      			case H_END_CR: {
        			if (str[i] == '\n') {
          				 m_hstate = H_END_LF;
        			} 
				else return PARSE_HEADER_ERROR;
        			break;
      			}
      			case H_END_LF: {
        			notFinish = false;
        			key_start = i;
        			now_read_line_begin = i;
        			break;
      			}
    		}
  	}
  	if (m_hstate == H_END_LF) {
    		str = str.substr(i);
    		return PARSE_HEADER_SUCCESS;
  	}
  	str = str.substr(now_read_line_begin);
  	return PARSE_HEADER_AGAIN;
}



AnalysisState HttpData::analysisRequest() {
	if (m_method == METHOD_POST) {}

	else if (m_method == METHOD_GET || m_method == METHOD_HEAD) {
    		string header;
    		header += "HTTP/1.1 200 OK\r\n";
    		if (m_headers.find("Connection") != m_headers.end() && (m_headers["Connection"] == "Keep-Alive" || m_headers["Connection"] == "keep-alive")) {
     	 		m_keep_alive = true;
      			header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    		}

    		int dot_pos = m_file_name.find('.');
    		string filetype;
    		if (dot_pos < 0)
      			filetype = MimeType::getMime("default");
    		else
     	 		filetype = MimeType::getMime(m_file_name.substr(dot_pos));


    		if (m_file_name_ == "hello") {
      			m_out_buffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
      			return ANALYSIS_SUCCESS;
    		}
    		if (m_file_name == "favicon.ico") {
     	 		header += "Content-Type: image/png\r\n";
      			header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
      			header += "Server: LinYa's Web Server\r\n";

      			header += "\r\n";
      			m_out_buffer += header;
      			m_out_buffer += string(favicon, favicon + sizeof favicon);
      			return ANALYSIS_SUCCESS;
    		}

    		struct stat sbuf;
    		if (stat(m_file_name.c_str(), &sbuf) < 0) {
      			header.clear();
      			handleError(m_fd, 404, "Not Found!");
      			return ANALYSIS_ERROR;
    		}
    		header += "Content-Type: " + filetype + "\r\n";
    		header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
    		header += "Server: LinYa's Web Server\r\n";

   	 	// 头部结束
    		header += "\r\n";
    		m_out_buffer += header;
    		if (method_ == METHOD_HEAD) return ANALYSIS_SUCCESS;

		//打开请求的文件
    		int src_fd = open(m_file_name.c_str(), O_RDONLY, 0);
    		if (src_fd < 0) {
      			m_out_buffer.clear();
      			handleError(m_fd, 404, "Not Found!");
      			return ANALYSIS_ERROR;
    		}
		//地址映射
    		void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    		close(src_fd);
    		if (mmapRet == (void *)-1) {
      			munmap(mmapRet, sbuf.st_size);
      			m_out_buffer.clear();
      			handleError(m_fd, 404, "Not Found!");
      			return ANALYSIS_ERROR;
    		}

    		char *src_addr = static_cast<char *>(mmapRet);
    		m_out_buffer += string(src_addr, src_addr + sbuf.st_size);
    		munmap(mmapRet, sbuf.st_size);
    		return ANALYSIS_SUCCESS;
  	}
  	return ANALYSIS_ERROR;
}



void HttpData::newEvent() {
	m_channel -> setEvents(DEFAULT_EVENT);
  	m_loop -> addToPoller(m_channel, DEFAULT_EXPIRED_TIME);
}

