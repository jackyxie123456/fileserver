#ifdef WIN32
#include <windows.h> //winÍ·ÎÄ¼þ
#endif 

#include <stdio.h>  
#include <string.h>    

#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "TaskQueue.h"
#include "Task.h"
#include "utils.h"
#include "logger.h"
#include "mergepipe.h"


#define TASK_NUM 1

bool IsQueueEmpty(TaskQueue** taskQueue ,int queueNo) {
	int iTaskQueue = 0;

	for (iTaskQueue = 0; iTaskQueue < queueNo; iTaskQueue++) {

		if (taskQueue[iTaskQueue]->GetTaskNo() != 0)
			return false;
	}
	return true;
}

typedef struct  __TaskContext {
	TaskQueue** taskQueues;
	int nQueueNo;
	int nCurr;
}TaskContext;

void* pipethread(void* pParam) {
	TaskContext* taskContext = (TaskContext*)pParam;

	while (1) {
		int nTaskIdx = taskContext->nCurr;
		TaskQueue* taskQueue = taskContext->taskQueues[nTaskIdx];
		readpipe(pipefd(), taskQueue);

		taskContext->nCurr++;
		if (taskContext->nCurr >= taskContext->nQueueNo)
			taskContext->nCurr = 0;

	}

}

extern void checkMerge(const char* path, TaskQueue* taskQueue1);

int main(int argc, char** argv) {

	TaskQueue* commonTaskQ[TASK_NUM] = {0};
	pthread_t waitTask;
	int iTask = 0;//

#ifdef WIN32
	const char* sFilePath = "d:/H264/";
#else
	const char* sFilePath = "/data/video/";
#endif
	log_set_file_prefix("mergedaemon-");

	log_info("{%s:%d} mergeDaemon start ...  "
		"\r\n",
		__FUNCTION__, __LINE__);

	int iTaskQ = 0;
	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		commonTaskQ[iTaskQ] = new TaskQueue();
		commonTaskQ[iTaskQ]->Start();
	}

	openpipe();//

	log_info("{%s:%d} mergeDaemon open pipe ...  "
		"\r\n",
		__FUNCTION__, __LINE__);

	TaskContext taskContext;
	taskContext.nCurr = 0;
	taskContext.nQueueNo = TASK_NUM;
	taskContext.taskQueues = commonTaskQ;
	pthread_create(&waitTask, NULL, pipethread, (void*)&taskContext);
	
	//
	time_t it;
	int t2 = time(0), t3;
	int tt01 = time(0), tt02 = 0;
	
	while (1) {
		
		t3 = time(0);
		if (t3 - t2 >= 60)
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
			
			std::cout << "main Thread Tick  " << ctime(&it) << std::endl;
			std::cout << taskMonitor;

			bool bUploadEmpty = IsQueueEmpty(commonTaskQ, TASK_NUM);
			if (bUploadEmpty) {
				
				/*
				log_info("{%s:%s:%d} mergeDaemon All task Is Finish ...   ",
					__FILE__, __FUNCTION__, __LINE__);
				*/
				//break;
			}
		}
		
		tt02 = time(0);
		if (tt02 - tt01 >= 120) {
			tt01 = tt02;//
			static int iTask = 0;
			log_info("{%s:%s:%d} mergeDaemon checkMerge......   ",
				__FILE__, __FUNCTION__, __LINE__);
			checkMerge(sFilePath , commonTaskQ[iTask++]);
			if (iTask >= TASK_NUM)
				iTask = 0;//
		}
		MSleep(50);
	}

	for (iTaskQ = 0; iTaskQ < TASK_NUM; iTaskQ++) {
		commonTaskQ[iTaskQ]->Stop();
		delete commonTaskQ[iTaskQ];//
	}

	log_info("{%s:%d} mergeDaemon exist ... at %s \r\n",
		__FUNCTION__, __LINE__, ctime(&it));

	return 0;
}


void MSleep(int nMs) {
#ifdef WIN32
	Sleep(nMs);
#else
	usleep(nMs * 1000);
#endif
}



