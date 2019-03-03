#ifndef AUTO_MUTEX__H_
#define AUTO_MUTEX__H_

#include "pthread.h"

#ifndef WIN32
#include <unistd.h>
#endif

extern void MSleep(int nMs);

class CMyMutex
{
public:
	CMyMutex()
	{
		//mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&mutex, NULL);
	}
	~CMyMutex() {
		pthread_mutex_destroy(&mutex);
	}
	void Lock()
	{
		pthread_mutex_lock(&mutex);
	}
	void Unlock()
	{
		pthread_mutex_unlock(&mutex);
	}


	int tryLock() {
		int nLockRt = 0;
		int iTime = 0;
		while (nLockRt = pthread_mutex_trylock(&mutex) != 0) {

			if (iTime == 100)
				break;
			MSleep(1);
		}

		return nLockRt;
	}
private:
	CMyMutex(const CMyMutex&){}
	pthread_mutex_t mutex;
};

class CLockGuard
{
public:
	CLockGuard(CMyMutex& mymutex)
		:m_mutex(mymutex)
	{
		m_mutex.Lock();
		//std::cout << "CLockGuard Lock" << std::endl;
	}

	virtual ~CLockGuard()
	{
		//std::cout <<"CLockGuard Release" << std::endl;
		m_mutex.Unlock();
	}
private:
	CLockGuard(const CLockGuard& lockguard)
		:m_mutex(lockguard.m_mutex){
	}
	
private:
	CMyMutex&  m_mutex;
};



#endif 