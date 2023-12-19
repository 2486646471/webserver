#include"LogStream.h"
#include<algorithm>

const char *digits = "0123456789";
template<typename T>
size_t convert(char buf[], T value) {
	T i = value;
	char* p = buf;

	while (i > 0) {
		int temp = static_cast<int>(i % 10);
		*p++ = digits[temp];
		i = i / 10;
	}

	if (value < 0) *p++ = '-';
	*p = '\0';
	std::reverse(buf, p);
	return p - buf;
}

template <typename T>
void LogStream::formatInteger(T v) {
	// buffer容不下kMaxNumericSize个字符的话会被直接丢弃
	if (m_buffer.avail() >= max_number_size) {
		size_t len = convert(m_buffer.current(), v);
		m_buffer.add(len);
  	}
}

LogStream& LogStream::operator<<(short v) {
	*this << static_cast<int>(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
  	*this << static_cast<unsigned int>(v);
  	return *this;
}

LogStream& LogStream::operator<<(int v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(long v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(long long v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(double v) {
	if (m_buffer.avail() >= max_number_size) {
    		int len = snprintf(m_buffer.current(), max_number_size, "%.12g", v);
    		m_buffer.add(len);
  	}
  	return *this;
}

LogStream& LogStream::operator<<(long double v) {
	if (m_buffer.avail() >= max_number_size) {
    		int len = snprintf(m_buffer.current(), max_number_size, "%.12Lg", v);
    		m_buffer.add(len);
  	}
  	return *this;
}
