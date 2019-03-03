#include "MergeTask.h"
#include "mergepipe.h"
#include "TaskQueue.h"
#include "logger.h"
#include <stdlib.h>
#include <memory.h>

#ifndef WIN32
#include<sys/types.h>
#include<fcntl.h>
#include<limits.h>
#endif

static int nfd = -1;

int openpipe() {
	
	const char*  pipe = PIPE_NAME;
#ifndef WIN32
	nfd = open(pipe, O_RDONLY);
#endif

	return (nfd > 0) ? 1 : 0;
}

int readpipe(int nPipeFd  , void* taskQueue) {
	MergeInfo mergeInfo;
	TaskQueue* taskQueue1 = (TaskQueue*)taskQueue;
	int nRead = 0;
	memset(&mergeInfo, 0x00, sizeof(mergeInfo));

#ifndef WIN32
	nRead = read(nPipeFd, &mergeInfo, sizeof(mergeInfo));
#endif

	if (nRead > 0) {
		std::string fileName(mergeInfo.fileName);
		int nTotal = mergeInfo.totalNo;
		std::string sPath;
		sPath = fileName.substr(0 , fileName.find_last_of("/\\"));

		MergeTask* mergeTask = new MergeTask(sPath, fileName , nTotal);
		
	
		if (taskQueue1) {
			if (!taskQueue1->findTask(mergeTask))
				taskQueue1->Put2Queue(mergeTask);
			else
				delete mergeTask;
		}
		else {
			delete mergeTask;
		}
		return 1;
	}

	return 0;
}

int pipefd() {
	return nfd;
}

void closepipe() {
#ifndef WIN32
	close(nfd);
#endif
	nfd = -1;
}

