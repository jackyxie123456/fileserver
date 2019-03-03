#pragma once

#include "TaskQueue.h"
#include "logger.h"

class MergeTask : public Task {
public:
	MergeTask(
		std::string sPath,
		std::string fileName , int nTotal ):
		m_sPath(sPath),
		m_sFileName(fileName),
		m_nTotal(nTotal)

	{
		int lastOfLimit = -1;
		lastOfLimit = fileName.find_last_of("/\\");
		
		if (-1 != lastOfLimit) {
			m_sFileName = fileName.substr(lastOfLimit + 1);
		}

		int nTotalAtStr = m_sFileName.find("_t_");

		if (std::string::npos != nTotalAtStr) {
			m_sFileName = m_sFileName.substr(0, nTotalAtStr);
		}

		log_info("{%s:%s:%d} MergeTask %s %s %d ",
			__FILE__, __FUNCTION__, __LINE__,
			m_sPath.c_str(), m_sFileName.c_str(),
			m_nTotal);
	}

	virtual ~MergeTask(){
	}

	virtual bool operator ==(const MergeTask& right) {
		
		
		if (m_nTotal == right.m_nTotal &&
			m_sPath.compare(right.m_sPath) == 0 &&
			m_sFileName.compare(right.m_sFileName) == 0)
			return true;
		return false;

	}
	/*
		0  Success
		other  failed
	*/
	virtual int Run();

	virtual std::string taskDescr() const{
		std::string taskDesc("");
		
		taskDesc += " Merge task ";
		taskDesc += ",szPath: " + m_sPath;
		taskDesc += ",szFile: " + m_sFileName;
		taskDesc += "\r\n";
		return taskDesc;
	}

	virtual void FinishCb() {
		
		log_info("{%s:%s:%d} MergeTask %s Finish @%s  ",
			__FILE__, __FUNCTION__, __LINE__, taskDescr().c_str(),
			getCurTime().c_str()) ;
		delete this;
	}

protected:
	std::string  m_sPath;
	std::string  m_sFileName;
	
	int  m_nTotal;
	
};

