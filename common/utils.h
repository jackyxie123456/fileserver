#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef WIN32
#include <wchar.h>   
#include <locale.h>   
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_PATH 
#define MAX_PATH 260
#endif

#define LF                  (u_char) '\n'
#define CR                  (u_char) '\r'
#define CRLF                "\r\n"

	wchar_t* ansi_to_unicode(char* str);
	char* unicode_to_ansi(wchar_t* str);
	wchar_t* utf8_to_unicode(char* str);
	char* unicode_to_utf8(wchar_t* str);
	char* utf8_to_ansi(char* str);
	char* ansi_to_utf8(char* str);
	int file_exist(const char *file_name);
	int remove_file(char *file_name);
	char* file_ext(char* file_name);
	int  file_len(const char* file_name);
	char* root_path();
void set_root_path(const char* path);
char* log_path();
void set_log_path(const char* path);
	char* uint32_to_str(unsigned int n);

	char* local_file_list(char *path);

char* get_default_store_path();

void set_default_store_path(const char* path);

int dir_exist(char *file_name);

void mkdirs(char *muldir);


#ifdef __cplusplus
}
#endif
#endif