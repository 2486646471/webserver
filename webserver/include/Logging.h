#pragma once
#include<string>
#include"LogStream.h"
#include<string>
#include<string.h>

class Logger {
public:
	Logger(const char *file_name, int line);
  	~Logger();
  	LogStream &stream() { return m_impl.m_stream; }

  	static void setLogFileName(std::string file_name) { m_log_file_name = file_name; }
  	static std::string getLogFileName() { return m_log_file_name; }
private:
	class Impl {
  	public:
   		Impl(const char *file_name, int line);
   		void formatTime();

   		LogStream m_stream;
   		int m_line;
   		std::string m_file_name;
 	};
	Impl m_impl;
  	static std::string m_log_file_name;
};

#define LOG Logger(__FILE__, __LINE__).stream()
