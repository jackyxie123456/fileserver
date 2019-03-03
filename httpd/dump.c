#include "dump.h"
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include "logger.h"
#include "utils.h"

static Except_frame* except_stack = NULL;

#define MAX_LEVEL 200 
#define SIZE_T_ 1024

#ifndef MAX_PATH 
#define MAX_PATH 260 
#endif 

void errorDump()
{
	void* buffer[MAX_LEVEL] = { 0 };
	char szDumpFile[MAX_PATH] = { 0 };
	int level = backtrace(buffer, MAX_LEVEL);
	int i = 0;//
	char cmd[SIZE_T_] = "addr2line -C -f -e ";
	char* prog = cmd + strlen(cmd);
	FILE* fp = NULL;
	char* pEnd = NULL;
	readlink("/proc/self/exe", prog, sizeof(cmd) - (prog - cmd) - 1);
	
	pEnd = cmd + strlen(cmd);
	sprintf(szDumpFile, "%shello.txt" , log_path());
	strcat(pEnd, szDumpFile);
	fp = popen(cmd, "w");
	if (!fp)
	{
		//perror("popen");
		return;
	}
	for (i = 0 ; i < level; ++i)
	{
		fprintf(fp, "%p\n", buffer[i]);
	}
	fclose(fp);
}


void recvSignal(int sig)
{
	printf("received signal %d !!!\n", sig);
	errorDump();
	siglongjmp(except_stack->env, 1);
}



void clearFrame(Except_frame* pFrame) {
	pFrame->flag = 0;
	memset(pFrame->env, 0x00, sizeof(pFrame->env));
}

Except_frame* getFrame() {
	if (NULL == except_stack) {
		except_stack = (Except_frame*)malloc(sizeof(Except_frame));
		if(NULL != except_stack)
			clearFrame(except_stack);
	}

	return except_stack;
}
