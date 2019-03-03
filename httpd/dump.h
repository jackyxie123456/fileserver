#ifndef _DUMP__H_
#define _DUMP__H_

#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "http.h"

#ifndef WIN32
#include <execinfo.h>
#endif
#ifdef __cplusplus
extern "C"{
#endif 

typedef struct __Except_frame
{
	jmp_buf env;
	int flag;
}Except_frame;

extern void errorDump();
extern void recvSignal(int sig);
extern void clearFrame(Except_frame*);
extern Except_frame* getFrame();

#define TRY \
	Except_frame* except_stack = getFrame();\
    except_stack->flag = sigsetjmp(except_stack->env,1);\
    if(!except_stack->flag) \
    { \
      signal(SIGSEGV,recvSignal); \
      printf("start use TRY\n");

#define END_TRY \
    }\
    else\
    {\
	  printf("exception raise....\n");\
	  http_stop();\
	  clearFrame(except_stack);\
    }\
    printf("stop use TRY\n");
#define RETURN_NULL \
    } \
    else \
    { \
      clearFrame(except_stack);\
    }\
    return NULL;
#define RETURN_PARAM  { \
      clearFrame(except_stack);\
    }\
    return x;
#define EXIT_ZERO \
    }\
    else \
    { \
      clearFrame(except_stack);\
    }\
    exit(0);

#ifdef __cplusplus
}
#endif 

#endif


