#pragma once

#include "TaskQueue.h"
#include "logger.h"
#include "DelayTask.h"

class MergeDelayTask : public DelayTask {
public:
	MergeDelayTask(std::string fileName ,int nDelay):
		DelayTask(nDelay),
		m_sFileName(fileName)

	{
		int lastOfLimit = -1;
		lastOfLimit = fileName.find_last_of("/\\");
		
		if (std::string::npos != lastOfLimit) {
			m_sFileName = fileName.substr(lastOfLimit + 1);
			m_sPath = fileName.substr(0, lastOfLimit);
		}

		int nTotal = 1;//

		int nTotalAtStr = m_sFileName.find("_t_");
		int nTotalEnd = m_sFileName.find("_i_");

		if (std::string::npos != nTotalAtStr
			&&  std::string::npos != nTotalEnd) {
			std::string sTotal = m_sFileName.substr(nTotalAtStr + 3, nTotalEnd - ( nTotalAtStr + 3 ) );
			nTotal = atoi(sTotal.c_str());
		}

		if (std::string::npos != nTotalAtStr) {
			m_sFileName = m_sFileName.substr(0, nTotalAtStr);
		}
		m_nTotal = nTotal;
	}

	virtual ~MergeDelayTask(){
	}


	
	/*
		0  Success
		other  failed
	*/
	virtual int Run();

	virtual std::string taskDescr() const {
		std::string taskDesc("");
		
		taskDesc += " Merge task ";
		taskDesc += ",szPath: ";
		taskDesc += m_sPath ;
		taskDesc += ",szFile: ";
		taskDesc += m_sFileName;
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
	virtual bool equalTo(const Task &rhs) const
	{

		const MergeDelayTask * task1 = dynamic_cast<const MergeDelayTask *>(&rhs);

		if (task1 == NULL)
			return false;

		if (this == task1)
			return true;

		if (m_nTotal == task1->m_nTotal &&
			m_sPath.compare(task1->m_sPath) == 0 &&
			m_sFileName.compare(task1->m_sFileName) == 0) {

			return true;

		}
		return false;

	}
protected:
	std::string  m_sPath;
	std::string  m_sFileName;
	
	int  m_nTotal;
	
};

