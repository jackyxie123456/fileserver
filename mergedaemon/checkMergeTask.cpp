#define MAX_NO 65536
#include <iostream>
#include "utils2.h"
#include "MergeDelayTask.h"
#include "logger.h"

void checkMerge(const char* path  , TaskQueue* taskQueue1) {

	const int maxNo = MAX_NO;
	std::string localFiles[MAX_NO];
	int iFile = 0;
	int nFiles  = local_file_list1(path, localFiles , 0 , maxNo);

	for (iFile = 0;  iFile < nFiles; iFile++) {
		
		std::string oldFile = localFiles[iFile];
		
		MergeDelayTask*  mergeTask = new MergeDelayTask(oldFile , 30);

		if (taskQueue1  && mergeTask) {
			if (!taskQueue1->findTask(mergeTask)) {

				taskQueue1->Put2Queue(mergeTask);

				log_info("{%s:%s:%d} task No %d new Task %s ",
					__FILE__, __FUNCTION__, __LINE__ , 
					taskQueue1->GetTaskNo(),
					mergeTask->taskDescr().c_str());
			}
			else
				delete mergeTask;
		}
		else {
			delete mergeTask;
		}

	}


}