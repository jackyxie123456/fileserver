#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#ifndef BUFFER_UNIT 
#define BUFFER_UNIT 4096
#endif

#ifndef MAX_PATH 
#define MAX_PATH 260
#endif 

static void log_print(log_level_t lv, const char *msg);
static const char* log_file_name();

static short loglevel = LOG_LEVEL;

#define BUFFER_MAX 4096

void log_set_level(short level) {
	loglevel = level;
}

void log_debug(const char *fmt, ...)
{
    char buffer[BUFFER_MAX] = { 0 };
    va_list args;

    if (LOG_DEBU < loglevel)
        return;
    
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    log_print(LOG_DEBU, buffer);
}

void log_info(const char *fmt, ...)
{
    char buffer[BUFFER_MAX] = { 0 };
    va_list args;

    if (LOG_INFO < loglevel)
        return;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    log_print(LOG_INFO, buffer);
}

void log_warn(const char *fmt, ...)
{
    char buffer[BUFFER_MAX] = { 0 };
    va_list args;

    if (LOG_WARN < loglevel)
        return;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    log_print(LOG_WARN, buffer);
    //ASSERT(false);
}

void log_error(const char *fmt, ...)
{
    char buffer[BUFFER_MAX] = { 0 };
    va_list args;

    if (LOG_ERRO < loglevel)
        return;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    log_print(LOG_ERRO, buffer);
    //ASSERT(false);
}

void log_fatal(const char *fmt, ...)
{
	char buffer[BUFFER_MAX] = { 0 };
	va_list args;

	if (LOG_FATAL < loglevel)
		return;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
	va_end(args);

	log_print(LOG_FATAL, buffer);
	//ASSERT(false);
}

static void log_print(log_level_t lv, const char *msg)
{
    const char *str = NULL;
    const char *file_name = NULL;
    FILE* fp = NULL;

    switch (lv)
    {
    case LOG_DEBU:
        str = "[DEBU]";
        break;
    case LOG_INFO:
        str = "[INFO]";
        break;
    case LOG_WARN:
        str = "[WARN]";
        break;
    case LOG_ERRO:
        str = "[ERRO]";
        break;
	case LOG_FATAL:
		str = "[FATAL]";
		break;
	}

    if (SAVE_FILE)
    {
        // write to file
        file_name = log_file_name();
        fp = fopen(file_name, "ab");
        if (fp)
        {
            fwrite(str, 1, strlen(str), fp);
            fwrite(" ", 1, 1, fp);
            fwrite(msg, 1, strlen(msg), fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
        }
    }
    printf("%s %s\n", str, msg);
}

#define MAX_PREFIX  40
static char szFilePrefix[MAX_PREFIX] = {0};

void log_set_file_prefix(char* szFile) {
	if (NULL == szFile)
		return;

	int nLen = strlen(szFile);
	nLen = nLen > (MAX_PREFIX - 1) ? MAX_PREFIX - 1 : nLen;

	memcpy(szFilePrefix, szFile, nLen);
}

static const char* log_file_name()
{
    static char file_name[MAX_PATH] = {0};
    static struct tm last = {0};

    time_t t = time(0);
    struct tm now;
    now = *localtime(&t);

    if (last.tm_year == now.tm_year
        && last.tm_mon == now.tm_mon
        && last.tm_mday == now.tm_mday)
    {
        return file_name;
    }
    memcpy(&last, &now, sizeof(struct tm));
    memset(file_name, 0, sizeof(file_name));
    memcpy(file_name, log_path(), strlen(log_path()));
	if (strlen(szFilePrefix) > 0) {
		strcat(file_name, szFilePrefix);
	}
    strftime(file_name + strlen(log_path() + strlen(szFilePrefix)),
		     sizeof(file_name) - strlen(log_path() - strlen(szFilePrefix)),
			"%Y-%m-%d.log", &now);
    return file_name;
}