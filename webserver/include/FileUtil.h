#pragma once
#include<string>
#include<fstream>
#include"noncopyable.h"

class AppendFile : noncopyable{
public:
	explicit AppendFile(std::string filename);
	~AppendFile();
	void append(const char* logline, size_t len);
	void flush();
private :
	std::ofstream m_ofs;
	char m_buf[64 * 1024];
};
