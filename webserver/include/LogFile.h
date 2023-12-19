#pragma once
#include<string>
#include<memory>
#include"FileUtil.h"
#include"MutexLock.h"
#include"noncopyable.h"

class LogFile : noncopyable {
private:
	std::unique_ptr<MutexLock> m_mutex;
	std::unique_ptr<AppendFile> m_file;
	int m_count;
	const int m_num_to_flush;
	const std::string m_filename; 
	void append_unlocked(const char* logline, int len);
public:
	LogFile(const std::string& filename, int num_to_flush = 1024);
	~LogFile();
	void append(const char* logline, int len);
	void flush();
};
