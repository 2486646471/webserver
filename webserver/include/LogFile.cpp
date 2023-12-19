#include"LogFile.h"
using namespace std;

LogFile::LogFile(const std::string& filename, int num_to_flush)
	: m_filename(filename), 
	  m_num_to_flush(num_to_flush), 
	  m_count(0), 
	  m_mutex(new MutexLock){
	  m_file.reset(new AppendFile(filename));
}
LogFile:: ~LogFile(){}

void LogFile::append(const char* logline, int len) {
	//加锁
	MutexLockGuard(*m_mutex);
	append_unlocked(logline, len);
}

void LogFile::flush() {
	MutexLockGuard lock(*m_mutex);
	m_file -> flush();
}

void LogFile::append_unlocked(const char* logline, int len) {
	m_file -> append(logline, len);
  	++m_count;
  	if (m_count >= m_num_to_flush) {
    		m_count = 0;
    		m_file -> flush();
  	}
}


