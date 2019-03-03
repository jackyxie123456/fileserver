#include <stdio.h>
#include <signal.h>

#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>


#include <unistd.h>
#include <pthread.h>


#define MAX 10

void when_sigsegv();
void *thread(void *arg);
void do_backtrace();

jmp_buf env[MAX];
long m[MAX] = { 0 };

int main()
{
	pthread_t thd[MAX];
	int i;

	signal(SIGSEGV, when_sigsegv);

	for (i = 0; i < MAX; i++)
	{
		pthread_create(&thd[i], NULL, (void *)thread, (void *)(uint64_t)i);
	}

	sleep(1);

	while (1)
	{
		printf("--------------------");
	}

	return 0;
}

void when_sigsegv()
{
	do_backtrace();
	int i = find_index((long)pthread_self());
	printf("sigsegv...%d...\n", i);
	siglongjmp(env[i], MAX);
}

void *thread(void *arg)
{
	int r = sigsetjmp(env[(int)(uint64_t)arg], MAX);
	if (r == 0)
	{
		printf("This is %d thread, id is %x\n", arg, pthread_self());
		m[(int)(uint64_t)arg] = (long)pthread_self();
		int *s = 0;
		(*s) = 1;  //向0地址写1，产生段错误
	}
	else
	{

	}
	sleep(1);
}

int find_index(long f)
{
	int i;
	for (i = 0; i < MAX; i++)
	{
		if (m[i] == f)
			return i;
	}

	printf("System Error,Found none!");

	return -1;
}

void do_backtrace()
{
	void *array[100];
	char **strings;
	int size, i;

	size = backtrace(array, 100);
	strings = backtrace_symbols(array, size);

	printf("%p\n", strings);
	for (i = 0; i < size; i++)
		printf("sigsegv at :%p:%s\n", array[i], strings[i]);

	free(strings);
}