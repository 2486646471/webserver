#include"FileUtil.h"
#include<iostream>
#include<streambuf>
using namespace std;

//打开文件并设置缓冲区大小
AppendFile::AppendFile(std::string filename) {
	m_ofs.open(filename, ios::app);
	m_ofs.rdbuf() -> pubsetbuf(m_buf, sizeof(m_buf));
}

AppendFile::~AppendFile() {
	m_ofs.close();
}	

//将文件写入缓冲区
void AppendFile::append(const char* logline, size_t len) {
	m_ofs.write(logline, len);
}

//等到写满了
void AppendFile::flush() {
	m_ofs.flush();
}

