#pragma once
#include "Task.h"
#include <deque>
#include "MyMutex.h"
#include "pthread.h"




class TaskQueue {

public:
	TaskQueue() :
		m_bStop(true)
	{
		m_dQueue.clear();
	}

	virtual ~TaskQueue() {

	}

	void Put2Queue(Task* uptask) {
		CLockGuard guard(m_cMutex);
		
		m_dQueue.push_back(uptask);
	}

	void RemoveTask(Task* uptask) {
		CLockGuard guard(m_cMutex);

	}

	int GetTaskNo() {
		CLockGuard guard(m_cMutex);
		int nSize = m_dQueue.size();
		return nSize;
	}

	void DoTask();

	void Start();

	void Stop();

	bool findTask(Task* task) {
		CLockGuard guard(m_cMutex);
		int iQueue = 0;
		for (iQueue = 0; iQueue < m_dQueue.size(); iQueue++) {
			Task* inqueTask = m_dQueue[iQueue];
			if (*inqueTask == *task) {
				return true;
			}
		}

		return false;
	}
private:
	TaskQueue(const TaskQueue&) {

	}
protected:
	std::deque<Task*> m_dQueue;
	CMyMutex m_cMutex;
	bool m_bStop;

	pthread_t m_uploadTask_t;


};


