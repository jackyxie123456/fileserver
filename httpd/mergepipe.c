#include "mergepipe.h"

#ifndef WIN32
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<limits.h>
#endif

static int nPipeFd = -1;

int openpipe() {
	const char *fifo_name = PIPE_NAME;
	int pipe_fd = -1;

	int res = 0;
	const int open_mode = O_WRONLY;
	
	if (access(fifo_name, F_OK) == -1)
	{
		res = mkfifo(fifo_name, 0777);
		if (res != 0)
		{
			fprintf(stderr, "could not create fifo\n");
			exit(-1);
		}
	}
	printf("process %d opening fifo O_WRONLY\n", getpid());


	nPipeFd = open(fifo_name, open_mode);

	return nPipeFd > 0 ? 1 : 0;

}

void closepipe() {
	close(nPipeFd);

	nPipeFd = -1;
}

int writepipe(MergeInfo* mergeInfo)
{
	int nBytes = 0;
	int res = write(nPipeFd, mergeInfo, sizeof(MergeInfo));
	if (res == -1)
	{
		fprintf(stderr, "write error\n");
	}

	fprintf(stderr, "write error\n");
}
