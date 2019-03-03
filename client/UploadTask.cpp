#include <string>
#include <iostream>
#include "utils.h"
#include "TaskQueue.h"
#include "UploadTask.h"
#include "UploadMoniterTask.h"

#define FILE_PART_SZ  (4 * 1024 * 1024) // 4M

extern int upload(std::string url,
	std::string fileName,
	std::string* response,int nTimeout = 1800);

int upload_part(std::string url,
	std::string fileName,
	std::string* response,
	int nPartSize,
	int nIndex,
	int nTimeout = 180);

int UploadTask::Run() {
	std::string response;

	log_info("{%s:%s:%d} task  %d %s  Running @ %s \r\n",
		__FILE__, __FUNCTION__, __LINE__,m_iId,taskDescr().c_str(),
		getCurTime().c_str());
	int status_code = 0;//
	if (m_bMultiPart) {
		// 已分片
		status_code =  upload_part(m_sUrl, m_sFile , &response, FILE_PART_SZ, m_nIndex);
		
		log_info("{%s:%s:%d} task Running %d i %d total %d,return %d \r\n %s ",
			__FILE__, __FUNCTION__, __LINE__, m_iId , m_nIndex , 
			m_nTotal, status_code, 
			response.c_str());

		return status_code;
	}

	int nTotal = checkMultiUpload();

	if (nTotal > 1) {
		// 多于一个分片
		int iIndex = 0; 
		UpMonitorTask* upMonitorTask = new UpMonitorTask(m_sFile , nTotal,m_nSize);

		if (NULL == upMonitorTask) {
			log_error("{%s:%s:%d} Error Run task Spilit  new UpMonitorTask Failed\n",
				__FILE__, __FUNCTION__, __LINE__,
				iIndex, nTotal);
			return -1;
		}
		
		upMonitorTask->SetHook(m_pHook, m_pObject);
		if (m_pTaskQueue) {
			m_pTaskQueue->Put2Queue(upMonitorTask);
		}
		else {
			log_error("{%s:%s:%d} Error Run task Spilit TaskQueue is NULL \n",
				__FILE__, __FUNCTION__, __LINE__,
				iIndex, nTotal);
			return -1;
		}
	
		for (iIndex = 0; iIndex < nTotal; iIndex++) {
			UploadTask* uploadTask = new UploadTask(m_sFile , m_sUrl);

			if (NULL == uploadTask)
				continue;

			log_info("{%s:%s:%d}  Spilit %s Task i %d , total:%d %x \n",
				__FILE__, __FUNCTION__, __LINE__,
				m_sFile.c_str(),
				iIndex, nTotal , m_pTaskQueue);

			uploadTask->SetIndex(iIndex, nTotal);
			uploadTask->SetHook(MonitorHook, upMonitorTask);
			if (m_pTaskQueue) {
				m_pTaskQueue->Put2Queue(uploadTask);
			}
			else {
				delete uploadTask;
			
				log_error("{%s:%s:%d} Error Run task Spilit Task i %d , total:%d \n",
					__FILE__, __FUNCTION__, __LINE__,
					iIndex, nTotal);
			}
		}

		SetHook(NULL, NULL);

		log_info("{%s:%s:%d} task Running %d ,big file spilit suc \r\n ",
			__FILE__, __FUNCTION__, __LINE__, m_iId);

		return 0;

	}
	else {
		status_code = upload(m_sUrl,
			m_sFile,
			&response);

		log_info("{%s:%s:%d} task Running %d ,return %d %s\r\n ",
			__FILE__, __FUNCTION__, __LINE__, m_iId, status_code, response.c_str());

	}
	 
	
	return status_code;
}

int UploadTask::checkMultiUpload() {

	int nFileLen = file_len(m_sFile.c_str());
	int nTotal = 1;

	if(nFileLen%FILE_PART_SZ == 0)
		nTotal = nTotal = nFileLen / FILE_PART_SZ;
	else
		nTotal = nFileLen / FILE_PART_SZ + 1;

	m_nSize = nFileLen;
	m_nTotal = nTotal;
	return nTotal;
}


 int UploadTask::m_iShardId = 0;
