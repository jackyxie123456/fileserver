#include <string>
#include <iostream>
#include "utils.h"
#include "TaskQueue.h"
#include "UploadMoniterTask.h"
#include "UploadTask.h"


int UpMonitorTask::Run() {
	std::string response;

	log_info("{%s:%s:%d} UpMonitorTask  %s  Running @ %s \r\n",
		__FILE__, __FUNCTION__, __LINE__,taskDescr().c_str(),
		getCurTime().c_str());
	int iFile = 0;//
	for (iFile = 0; iFile < m_nTotal; iFile++) {
		if (m_bIdxFinish && false == m_bIdxFinish[iFile])
			return -2;//
	}

	return 0;// finish
}

void UpMonitorTask::SetIdxFinish(int idx) {
	if (idx < m_nTotal  && NULL != m_bIdxFinish) {
		m_bIdxFinish[idx] = true;
	}

	log_info("{%s:%s%d}  UpMonitorTask %s setIdx %d finish ", 
		__FILE__,
		__FUNCTION__,
		__LINE__,
		m_sFile.c_str(), idx);
}	

void  MonitorHook(Task* task, void* pObject) {
	
	UploadTask* uploadTask = dynamic_cast<UploadTask*>(task);

	UpMonitorTask* upMoniterTask = (UpMonitorTask*)(pObject);

	if (nullptr == uploadTask || upMoniterTask == NULL) {
		return;//
	}
	

	upMoniterTask->SetIdxFinish(uploadTask->GetIndex());



}

