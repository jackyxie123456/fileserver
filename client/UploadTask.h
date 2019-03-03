#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include "logger.h"
#include "Task.h"



#ifdef WIN32
#define DEFALUT_FILE "d:\\H264\\slamtv60.264"
#else
#define DEFALUT_FILE "/root/slamtv60.264"
#endif 

class TaskQueue;

class  UploadTask : public Task{
public:
	UploadTask(std::string fileName = DEFALUT_FILE,
		std::string url = "http://127.0.0.1/upload?path=") :
		m_sFile(fileName),
		m_sUrl(url),
		m_iId(0),
		m_isFinish(false),
		m_bMultiPart(false),
		m_nIndex(0),
		m_nTotal(1)
	{
		m_iId = m_iShardId++;
	}

	virtual int Run();

	void SetIndex(int nIndex, int nTotal) {
		m_bMultiPart = true;
		m_nIndex = nIndex;
		m_nTotal = nTotal;
	}

	void FinishCb() {

		log_info("{%s:%s:%d} task  %d %s  Finish @%s Exec :%d \r\n",
			__FILE__, __FUNCTION__, __LINE__, m_iId ,
			taskDescr().c_str() , getCurTime().c_str() , 
			m_nExecEnd - m_nExecStart);

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

		if (m_bMultiPart) {
			std::stringstream sDesc;

			sDesc  << "task: " + m_sUrl + "\r\n"
				+ ",File:" + m_sFile;
			sDesc << ",i ";
			sDesc << m_nIndex;
			sDesc << ",total " << m_nTotal;
			sDesc <<   "\r\n";

			return sDesc.str();
		}

		return "task: " + m_sUrl + "\r\n" 
			+ ",File:"+ m_sFile + "\r\n";
	}

	std::string GetFileName() {
		return m_sFile;
	}

	int GetIndex() {
		return m_nIndex;
	}

	int GetTotal() {
		return m_nTotal;
	}

	bool GetMultiPart() {
		return m_bMultiPart;
	}

	int GetSize() {
		return m_nSize;
	}
protected:
	UploadTask(const UploadTask&) {

	}

	int m_iId;
	bool m_isFinish;
	std::string m_sUrl;
	std::string m_sFile;

	bool m_bMultiPart;
	int  m_nIndex;
	int  m_nTotal;
	int  m_nSize;

	int checkMultiUpload();
protected:

protected:
	static int m_iShardId;

};