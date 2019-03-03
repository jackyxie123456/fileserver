#include "httpd.h"
#include "mergepipe.h"

#ifdef WIN32
#include <Dbghelp.h> 

#pragma comment(lib, "Dbghelp.lib")

LONG __stdcall crush_callback(struct _EXCEPTION_POINTERS* ep)
{
    time_t t;
    struct tm *p;
    char fname[MAX_PATH] = {0};
    MINIDUMP_EXCEPTION_INFORMATION    exceptioninfo;
    HANDLE hFile;

    t = time(NULL) + 8 * 3600;
    p = gmtime(&t);

    sprintf(fname, "dump_%d-%d-%d_%d_%d_%d.DMP", 
		1900 + p->tm_year,
		1 + p->tm_mon, 
		p->tm_mday, 
		(p->tm_hour)%24, p->tm_min, p->tm_sec);

    hFile = CreateFileA(fname,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    
    exceptioninfo.ExceptionPointers = ep;
    exceptioninfo.ThreadId          = GetCurrentThreadId();
    exceptioninfo.ClientPointers    = FALSE;

    if (!MiniDumpWriteDump(GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpWithFullMemory,
        &exceptioninfo,
        NULL,
        NULL))
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include "logger.h"
#include "dump.h"

void crush_callback()
{
	char buf[4096] = { 0 };
	char cmd[1024] = { 0 };
	FILE * pFile = 0;
	time_t t = time(0);
	struct tm now;
	int i = 0;//
	int nSize = 0;//
	void * array[100] = { 0 }; /* 100 层，太够了 : )，你也可以自己设定个其他值 */


	now = *localtime(&t);
	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
	if (!(pFile = fopen(buf, "r")))
		return;
	if (!fgets(buf, sizeof(buf), pFile))
		return;
	fclose(pFile);

	if (buf[strlen(buf) - 1] == '/n')
		buf[strlen(buf) - 1] = '/0';

	snprintf(cmd, sizeof(cmd),
		"gdb %s %d -ex=bt > ./%02d_%02d_%02d.txt", buf, getpid(),
		now.tm_hour, now.tm_min, now.tm_sec);
	system(cmd);

	
	nSize = backtrace(array, sizeof(array) / sizeof(array[0]));

	for (i = nSize - 3; i >= 2; i--) {
		/* 头尾几个地址不必输出，看官要是好奇，输出来看看就知道了 */

		/* 修正array使其指向正在执行的代码 */

		log_warn("SIGSEGV  running code at %x\n", (char*)(array[i] - 1));

	}

	printf("handle sigv ......\r\n");
}


#endif

int main(int argc ,char** argv)
{
	uint16_t port = 80;
#ifdef WIN32
    SetUnhandledExceptionFilter(crush_callback);
#else 
	TRY 
	END_TRY
#endif 
	if (argc > 1) {
		set_root_path(argv[1]);
	}

	if (argc > 2) {
		set_log_path(argv[2]);
	}

	init_CRC32_table();

	openpipe();

    http_startup(&port);
    
	system("pause");
    
	return 0;
}



