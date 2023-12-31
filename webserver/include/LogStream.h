#pragma once
#include<string.h>
#include<string>
#include"noncopyable.h"

const int small_buffer = 4000;
const int big_buffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : noncopyable{
public:
	FixedBuffer():m_cur(m_data){}
	~FixedBuffer(){}
	int avail() const {
		return static_cast<int>(end() - m_cur);
	}
	void append(const char* buf, size_t len) {
		if (avail() > static_cast<int>(len)) {
			memcpy(m_cur, buf, len);
			m_cur += len;
		}
	}

	const char* data() const {return m_data; }
	int length() const { return static_cast<int>(m_cur - m_data); }
	void reset() { m_cur = m_data; }
  	void bzero() { memset(m_data, 0, sizeof(m_data));}
	char* current() {return m_cur;}
	void add(size_t len) {m_cur += len;}
private:
	char* m_cur;
	char m_data[SIZE];
	const char* end() const {
		return m_data + sizeof(m_data);
	}
};

class LogStream : noncopyable {
public:
	typedef FixedBuffer<small_buffer> Buffer;

	LogStream& operator<<(bool v) {
		m_buffer.append(v ? "1" : "0", 1);
		return *this;
  	}
	LogStream& operator<<(short);
	LogStream& operator<<(unsigned short);
	LogStream& operator<<(int);
	LogStream& operator<<(unsigned int);
 	LogStream& operator<<(long);
 	LogStream& operator<<(unsigned long);
 	LogStream& operator<<(long long);
 	LogStream& operator<<(unsigned long long);

	LogStream& operator<<(float v) {
		*this << static_cast<double>(v);
    		return *this;
  	}

	LogStream& operator<<(double);
  	LogStream& operator<<(long double);

  	LogStream& operator<<(char v) {
		m_buffer.append(&v, 1);
    		return *this;
  	}
	LogStream& operator<<(const char* str) {
		if (str)
      			m_buffer.append(str, strlen(str));
    		else
      			m_buffer.append("(null)", 6);
    		return *this;
  	}
	LogStream& operator<<(const std::string& v) {
    		m_buffer.append(v.c_str(), v.size());
    		return *this;
  	}

	void append(const char* data, int len) { m_buffer.append(data, len); }
  	const Buffer& buffer() const { return m_buffer; }
  	void resetBuffer() { m_buffer.reset(); }

private:
	template <typename T>
  	void formatInteger(T);
	Buffer m_buffer;
	static const int max_number_size = 32;
};
