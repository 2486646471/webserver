#include "AsyncLogging.h"
#include "LogFile.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>

AsyncLogging::AsyncLogging(std::string file_name, int flush_interval)
	: m_flush_interval(flush_interval),
	m_running(false),
	m_name(file_name),
	m_thread(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      	m_mutex(),
	m_cond(m_mutex),
	current_buffer(new Buffer),
        next_buffer(new Buffer),
	buffers(),
        m_latch(1) {
		assert(file_name.size() > 1);
		//清空缓冲区
  		current_buffer -> bzero();
  		next_buffer -> bzero();
		//提前分配内存
  		buffers.reserve(16);
}


void AsyncLogging::append(const char* logline, int len) {
	//对缓冲区操作要进行加锁
	MutexLockGuard lock(m_mutex);

	//如果当前缓冲区空间足够
	if (current_buffer -> avail() > len)
    		current_buffer -> append(logline, len);
  	else {
    		buffers.push_back(current_buffer);

		//将指针置空
    		current_buffer.reset();

		//如果nextbuffer不为空
    		if (next_buffer)
      			current_buffer = std::move(next_buffer);
    		else
      			current_buffer.reset(new Buffer);
    		current_buffer -> append(logline, len);
		//缓冲区满了，通知
    		m_cond.notify();
  	}
}

void AsyncLogging::threadFunc() {
	assert(m_running == true);
  	m_latch.countDown();
	LogFile output(m_name);

  	BufferPtr newBuffer1(new Buffer);
  	BufferPtr newBuffer2(new Buffer);
  	newBuffer1 -> bzero();
        newBuffer2 -> bzero();
        BufferVector buffersToWrite;
        buffersToWrite.reserve(16);

	while (m_running) {
		assert(newBuffer1 && newBuffer1 -> length() == 0);
		assert(newBuffer2 && newBuffer2 -> length() == 0);
		assert(buffersToWrite.empty());

		{
			MutexLockGuard lock(m_mutex);
      			if (buffers.empty()) {
        			m_cond.waitForSeconds(m_flush_interval);
      			}
      			buffers.push_back(current_buffer);
      			current_buffer.reset();

      			current_buffer = std::move(newBuffer1);
      			buffersToWrite.swap(buffers);
      			if (!next_buffer) {
        			next_buffer = std::move(newBuffer2);
      			}
    		}
		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25) {
  			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}
				
		for (size_t i = 0; i < buffersToWrite.size(); ++i) {
      			output.append(buffersToWrite[i] -> data(), buffersToWrite[i] -> length());
    		}

    		if (buffersToWrite.size() > 2) {
      			buffersToWrite.resize(2);
    		}

  		if (!newBuffer1) {
    			assert(!buffersToWrite.empty());
    			newBuffer1 = buffersToWrite.back();
			//避免使用同一个缓冲区，就弹出
    			buffersToWrite.pop_back();
    			newBuffer1 -> reset();
  		}

  		if (!newBuffer2) {
    			assert(!buffersToWrite.empty());
    			newBuffer2 = buffersToWrite.back();
    			buffersToWrite.pop_back();
    			newBuffer2->reset();
  		}

  		buffersToWrite.clear();
  		output.flush();
	}
	output.flush();
}




