#include"Logging.h"
#include "Thread.h"
#include "AsyncLogging.h"
#include <time.h>
#include <sys/time.h>

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger;

std::string Logger::m_log_file_name = "WebServer.log";

void once_init(){
	AsyncLogger = new AsyncLogging(Logger::getLogFileName());
	AsyncLogger -> start(); 
}

//调用AsyncLogging,执行异步写入日志。并且As
void output(const char* msg, int len){
	//保证once_init函数只是调用一次
	pthread_once(&once_control, once_init);
	AsyncLogger -> append(msg, len);
}


Logger::Impl::Impl(const char *file_name, int line)
	: m_stream(),
	m_line(line),
	m_file_name(file_name){
		//在Impl初始化的时候，就将时间写入缓冲区
    		formatTime();
}

//获取当前时间，格式化为字符串
void Logger::Impl::formatTime(){
	struct timeval tv;
    	time_t time;
    	char str_t[26] = {0};

    	gettimeofday (&tv, NULL);
    	time = tv.tv_sec;
    	struct tm* p_time = localtime(&time);
    	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    	m_stream << str_t;
}


Logger::Logger(const char *file_name, int line): m_impl(file_name, line){}


Logger::~Logger()
{
    m_impl.m_stream << " -- " << m_impl.m_file_name << ':' << m_impl.m_line << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    //在对象析构的时候,将调用的cpp，以及行号写入
    output(buf.data(), buf.length());
}

