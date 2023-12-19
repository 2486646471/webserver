#pragma once


//定义了对http请求处理的不同阶段
enum ProcessState {
	STATE_PARSE_URI = 1,
	STATE_PARSE_HEADERS,
	STATE_RECV_BODY,
	STATE_ANALYSIS,
	STATE_FINISH
};

//解析uri的状态
enum URIState {
	PARSE_URI_AGAIN = 1,
  	PARSE_URI_ERROR,
  	PARSE_URI_SUCCESS,
};

//解析头部的状态
enum HeaderState {
	PARSE_HEADER_SUCCESS = 1,
  	PARSE_HEADER_AGAIN,
  	PARSE_HEADER_ERROR
};

//分析请求的状态
enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

enum ParseState {
	H_START = 0,
  	H_KEY,
  	H_COLON,
  	H_SPACES_AFTER_COLON,
  	H_VALUE,
  	H_CR,
  	H_LF,
  	H_END_CR,
  	H_END_LF
};

//连接的状态
enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };

enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

//表示HTTP的版本
enum HttpVersion { HTTP_10 = 1, HTTP_11 };

class MimeType {
private:
	static void init();
  	static std::unordered_map<std::string, std::string> mime;
  	MimeType();
  	MimeType(const MimeType &m);

public:
  	static std::string getMime(const std::string &suffix);

 private:
  	static pthread_once_t once_control;
};


class HttpData : public std::enable_shared_from_this<HttpData> {
private:
	EventLoop *m_loop;
  	std::shared_ptr<Channel> m_channel;
  	int m_fd;
  	std::string m_in_buffer;	//存储从客户端接收的数据
  	std::string m_out_buffer;	//存储向客户端发送的数据
  	bool m_error;
  	ConnectionState m_connection_state;

	HttpMethod m_method;
	HttpVersion m_http_version;
	std::string m_file_name;
	std::string m_path;
	int m_read_pos;
	ProcessState m_state;
 	ParseState m_hstate;
	bool m_keep_alive;
	std::map<std::string, std::string> m_headers;
	std::weak_ptr<TimerNode> m_timer;

	void handleRead();
  	void handleWrite();
  	void handleConn();
  	void handleError(int fd, int err_num, std::string short_msg);
  	URIState parseURI();
  	HeaderState parseHeaders();
  	AnalysisState analysisRequest();

public:
	HttpData(EventLoop* loop, int connfd);
  	~HttpData() { close(m_fd); }
  	void reset();
  	void seperateTimer();
  	void linkTimer(std::shared_ptr<TimerNode> timer) {
    		m_timer = timer;
  	}
  	std::shared_ptr<Channel> getChannel() { return m_channel; }
  	EventLoop *getLoop() { return m_loop; }
  	void handleClose();
  	void newEvent();
};




