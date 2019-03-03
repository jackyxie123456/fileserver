#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include "logger.h"
#include "Task.h"


class TaskQueue;

class  UpMonitorTask : public Task{
public:
	UpMonitorTask(std::string fileName ,int nTotal ,int nSize) :
		m_sFile(fileName),
		m_isFinish(false),
		m_nTotal(nTotal),
		m_nSize(nSize),
		m_bIdxFinish(NULL)
	{
		m_bIdxFinish = new bool[nTotal];
		if (m_bIdxFinish) {
			memset(m_bIdxFinish, 0x00, sizeof(bool) * nTotal);
		}
	}

	~UpMonitorTask() {
		delete[] m_bIdxFinish;
	}

	virtual int Run();

	void SetIdxFinish(int idx);

	void FinishCb() {

		log_info("{%s:%s:%d} UpMoniterTask  %s  Finish @%s  \r\n",
			__FILE__, __FUNCTION__, __LINE__ ,
			taskDescr().c_str() , getCurTime().c_str());

		m_isFinish = true;
		if (m_pHook) {
			(*m_pHook)(this , m_pObject);
		}

		delete this;
	}

	bool IsFinish() {
		return m_isFinish;
	}

	virtual std::string taskDescr() const {
		std::string taskDes;

		taskDes += "upMoniterTask " + std::string("\r\n");
		taskDes += ",File:" + m_sFile + "\r\n";
		
		return taskDes;
	}

	std::string  GetFileName() {
		return m_sFile;
	}
	int GetTotal() {
		return m_nTotal;
	}

	int GetSize() {
		return m_nSize;
	}
	
protected:
	UpMonitorTask(const UpMonitorTask&) {

	}

	bool m_isFinish;
	std::string m_sFile;

	int  m_nTotal;
	int  m_nSize;

protected:
	bool* m_bIdxFinish;

};

void  MonitorHook(Task*, void* pObject);