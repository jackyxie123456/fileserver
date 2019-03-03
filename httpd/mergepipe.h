#pragma once

#ifndef MAX_PATH 
#define MAX_PATH 260
#endif 

#define PIPE_NAME "/tmp/merge_fifo"

#ifdef __cplusplus
extern "C" {
#endif


	typedef struct __MergeInfo {
		char fileName[MAX_PATH];
		int  totalNo;
	}MergeInfo;

	int pipefd();
	int openpipe();
	int writepipe(MergeInfo* mergeInfo);



#ifdef __cplusplus
}
#endif