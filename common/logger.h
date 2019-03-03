#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    LOG_DEBU,
    LOG_INFO,
    LOG_WARN,
    LOG_ERRO,
	LOG_FATAL,
} log_level_t;

#define LOG_LEVEL   LOG_INFO
#define SAVE_FILE   1

void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_fatal(const char *fmt, ...);

void log_set_file_prefix(char* szFile);
void log_set_level(short loglevel);

#ifdef __cplusplus
}
#endif

#endif