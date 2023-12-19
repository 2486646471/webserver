#pragma once
namespace CurrentThread {
	extern __thread int m_cache_tid;
	extern __thread char m_tid_string[32];
	extern __thread int m_tid_length;
	extern __thread const char* m_thread_name;

	void cacheTid();

	inline int tid() {	
		if (__builtin_expect(m_cache_tid == 0, 0)) {
			cacheTid();
		}
		return m_cache_tid;
	}
	inline char* tidString() {
		return m_tid_string;
	}

	inline int tidLength() {
		return m_tid_length;
	}
	inline const char* name() {
		return m_thread_name;
	}
}

