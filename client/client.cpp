#include <stdio.h>  
#include <string.h>    
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "TaskQueue.h"
#include "Task.h"
#include "utils.h"

#include "DbHelper.h"
#include "DbTask.h"

#include "logger.h"
#include "UploadTask.h"
#include "crc32.h"
#include "UploadMoniterTask.h"



#define TASK_NUM 4
#define MAX_NO 16384

int  checkBreak();
int local_file_list(const char *path, std::string* outfiles, int maxNo);

bool IsAllFinish(UploadTask** tasks , int taskNo) {

	bool isFinish = false;
	int iTask = 0;
	for (iTask = 0; iTask < taskNo; iTask++) {
		if (tasks[iTask] && false == tasks[iTask]->IsFinish())
			return false;
	}

	return true;
}

bool IsQueueEmpty(TaskQueue** taskQueue ,int queueNo) {
	int iTaskQueue = 0;

	for (iTaskQueue = 0; iTaskQueue < queueNo; iTaskQueue++) {

		if (taskQueue[iTaskQueue]->GetTaskNo() != 0)
			return false;
	}
	return true;
}

void UploadPostTask(Task* task, void* pObject) {

	TaskQueue* taskQ = (TaskQueue*)pObject;
	UploadTask* uploadTask = dynamic_cast<UploadTask*>(task);
	FileDbItem fileDbItem;
	int t3 = time(0);

	if (NULL == taskQ)
		return;

	if (nullptr != uploadTask) {
		strcpy(fileDbItem.szFileName, uploadTask->GetFileName().c_str());
		fileDbItem.isUpload = 1;
		fileDbItem.nCreateTime = t3;
		fileDbItem.nIndex = uploadTask->GetIndex();
		fileDbItem.nTotal = uploadTask->GetTotal();

		if (uploadTask->GetFileName().length() == 0) {
			log_error("{%s:%s:%d} UploadPostTask  Error fileName is null\n ",
				__FILE__, __FUNCTION__, __LINE__);

			return;
		}

		Task* dbTask = new DbTask(GetDbHelper(), fileDbItem);

		log_info("{%s:%s:%d} new DbTask %s \n ",
			__FILE__, __FUNCTION__, __LINE__, fileDbItem.szFileName);
		taskQ->Put2Queue(dbTask);
		return ;
	}
		
	UpMonitorTask * upMoniterTask = dynamic_cast<UpMonitorTask*>(task);
	
	if (nullptr != upMoniterTask) {
		strcpy(fileDbItem.szFileName, upMoniterTask->GetFileName().c_str());
		fileDbItem.isUpload = 1;
		fileDbItem.nCreateTime = t3;
		fileDbItem.nIndex = 0;
		fileDbItem.nTotal = upMoniterTask->GetTotal();
		fileDbItem.nSize = upMoniterTask->GetSize();

		Task* dbTask = new DbTask(GetDbHelper(), fileDbItem);

		log_info("{%s:%s:%d} new UpMonitorTask hook  %s %d %d \n ",
			__FILE__, __FUNCTION__, __LINE__, upMoniterTask->GetFileName() , 
			upMoniterTask->GetTotal() ,upMoniterTask->GetSize());
		taskQ->Put2Queue(dbTask);
		return;
	}
	
		
	
}


std::string files[MAX_NO] = {""};
UploadTask* tasks[MAX_NO] = { 0 };

int main(int argc, char** argv) {

	TaskQueue* commonTaskQ[TASK_NUM] = {0};
	TaskQueue* dbTaskQ[TASK_NUM] = { 0 };
	int iTask = 0;//

	char* sUrlPa = "http://%s/upload?path=";
	char sUrl[MAX_PATH] = { 0 };
	
	char sWebsite[MAX_PATH] = "lucksensedev.luckincoffee.com";
	char sStoreName[] = "230060";

#ifdef WIN32
	char sDbPath[MAX_PATH] = "d:\\";
	char sFilePath[MAX_PATH] = "d:\\H264\\";
#else 
	char sDbPath[MAX_PATH] = "/root/";
	char sFilePath[MAX_PATH] = "/root/client/";
#endif

	if (argc > 4) {
		sprintf(sWebsite, "%s", argv[1]);
		sprintf(sStoreName, "%s", argv[2]);
		sprintf(sFilePath, "%s", argv[3]);
		sprintf(sDbPath, "%s", argv[4]);
	}
	else if (argc > 3) {
		sprintf(sWebsite, "%s", argv[1]);
		sprintf(sStoreName, "%s", argv[2]);
		sprintf(sFilePath, "%s", argv[3]);
	}
	else if (argc > 2) {
		sprintf(sWebsite, "%s", argv[1]);
		sprintf(sStoreName, "%s", argv[2]);
	}
	else {
		printf("Usage : httpsync.exe serverip storeId rootdir dbdir");
		return -1;
	}

	init_CRC32_table();

	sprintf(sUrl, sUrlPa, sWebsite);
	
	log_set_file_prefix("httpsync");

	log_info("{%s:%d} httpsync start ... remote : %s , "
		"storeId:%s , filedir : %s ,db: %s \r\n", 
		__FUNCTION__, __LINE__,
		sWebsite , sStoreName , sFilePath, sDbPath);

	int iTaskQ = 0;
	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		commonTaskQ[iTaskQ] = new TaskQueue();
		commonTaskQ[iTaskQ]->Start();
	}

	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		dbTaskQ[iTaskQ] = new TaskQueue();
		dbTaskQ[iTaskQ]->Start();
	}
	
	strcat(sUrl, "&storeId=");
	strcat(sUrl, sStoreName);

	std::string dbFile(sDbPath);
	dbFile += DB_NAME;

	DbHelper& dbHelper = GetDbHelper(dbFile.c_str());
	if (!file_exist((char*)dbFile.c_str())) {
		dbHelper.createTable();
		dbHelper.createIndex();
	}

	

	int nFiles = local_file_list(sFilePath, files, MAX_NO);
	
	for(iTask = 0; iTask < nFiles; iTask++){

		std::string fileName(files[iTask]);

		int iFound = 0;
		std::cout << "Fetch files " << fileName << std::endl;
		FileDbItem fileItem;
		
		iFound = dbHelper.FindByName(fileName , 0 , &fileItem);
		if (iFound == 0 || 
			(iFound > 0 && 0 == fileItem.isUpload) ) {
			UploadTask* uploadTask1 = new UploadTask(fileName,
				sUrl);

			if (NULL == uploadTask1)
				continue;

			uploadTask1->SetHook(UploadPostTask, (void*)dbTaskQ[iTask%TASK_NUM]);

			tasks[iTask] = uploadTask1;
			commonTaskQ[iTask%TASK_NUM]->Put2Queue(uploadTask1);
		}
	}

	//
	time_t it;
	int t2 = time(0), t3;
	
	while (1) {
		if (checkBreak() > 0) {
			break;
		}
		t3 = time(0);
		if (t3 - t2 >= 10)
		{
			time(&it);
			t2 = t3;

			std::string taskMonitor("");
			for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
				char taskInfo[512] = { 0 };
				sprintf(taskInfo, "task : %d ,TaskNo : %d ",
					iTaskQ, commonTaskQ[iTaskQ]->GetTaskNo());
				taskMonitor.append(taskInfo);
				taskMonitor.append(CRLF);
			}
			for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
				char taskInfo[512] = { 0 };
				sprintf(taskInfo, "task : %d ,TaskNo : %d ",
					iTaskQ, dbTaskQ[iTaskQ]->GetTaskNo());
				taskMonitor.append(taskInfo);
				taskMonitor.append(CRLF);
			}

			std::cout << "main Thread Tick  " << ctime(&it) << std::endl;
			std::cout << taskMonitor;


			bool bUploadEmpty = IsQueueEmpty(commonTaskQ, TASK_NUM);
			if (bUploadEmpty) {
				bool bDbEmpty  = IsQueueEmpty(dbTaskQ , TASK_NUM);
				if (bDbEmpty)
				{
					log_info("{%s:%s:%d} httpsync All task Is Finish ...   ",
						__FILE__, __FUNCTION__, __LINE__);
					break;
				}
			}
		}
		

		MSleep(50);
	}

	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		commonTaskQ[iTaskQ]->Stop();
		delete commonTaskQ[iTaskQ];//
	}

	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		dbTaskQ[iTaskQ]->Stop();
		delete dbTaskQ[iTaskQ];//
	}

	log_info("{%s:%d} httpsync exist ... at %s \r\n",
		__FUNCTION__, __LINE__, ctime(&it));

	return 0;
}

