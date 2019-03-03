#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include "logger.h"
#include "Task.h"

class TaskQueue;

class DelayTask :public Task{
public:
	/* nDelayTime  count in seconds */
	DelayTask(int nDelayTime = 1):
		m_nDelayTime(nDelayTime)
	{
		m_nCreate = time(0);
	}
	virtual ~DelayTask() {

	}

	/*
		0  Success
		other  failed
	*/
	virtual int Run() = 0;

	virtual std::string taskDescr() {
		return "";
	}

	virtual void FinishCb() {
		if (m_pHook) {
			m_pHook(this, m_pObject);
		}
	}


protected:

	virtual bool canExec() {
		int nCurrTime = time(0);
		if (nCurrTime > (m_nCreate +  nCurrTime ) ) {
			return true;
		}

		return false;
	}

	int m_nCreate;
	int m_nDelayTime;
	
protected:

};
