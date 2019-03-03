#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include "logger.h"



class TaskQueue;
class Task;
typedef void(*TaskHook)(Task*, void* pObject);

class Task {
public:
	Task():m_pTaskQueue(NULL),
		m_pHook(NULL),
		m_pObject(NULL),
		m_nExecStart(0),
		m_nExecEnd(0)
	{}
	virtual ~Task() {

	}

	/*
		0  Success
		other  failed
	*/
	virtual int Run() = 0;

	virtual std::string taskDescr() const = 0;

	virtual void FinishCb() = 0;

	

	inline bool operator==(const Task &rhs)
	{
		return equalTo(rhs) && rhs.equalTo(*this);
	}

	TaskQueue* taskQueue() {
		return m_pTaskQueue;
	}

	
	
	std::string getCurTime() {
		
		std::stringstream ssTime;
		time_t it(NULL);

		time(&it);
		ssTime << ctime(&it);
		
		return ssTime.str();
	}

	void SetHook(TaskHook pHook, void * pObject) {
		m_pHook = pHook;
		m_pObject = pObject;
	}

	TaskHook GetHook() {
		return m_pHook;
	}
	void* GetHookData() {
		return m_pObject;
	}

	virtual int execTime() {
		return m_nExecEnd - m_nExecStart;
	}

	void setQueue(TaskQueue* taskQueue) {
		m_pTaskQueue = taskQueue;
	}
protected:

	virtual bool equalTo(const Task &rhs) const
	{

		const Task * task1 = dynamic_cast<const Task *>(&rhs);

		if (task1 == NULL)
			return false;

		if (this == task1)
			return true;

		return false;

	}

	int m_nExecStart;
	int m_nExecEnd;
	friend class TaskQueue;

	void setExecStart(int time1) {
		m_nExecStart = time1;
	}
	void setExecEnd(int time1) {
		m_nExecEnd = time1;
	}
protected:
	TaskHook m_pHook;
	void* m_pObject;
	TaskQueue* m_pTaskQueue;
};
