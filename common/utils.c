

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#else 
#include <Windows.h>
#endif

#include "utils.h"


#ifndef BUFFER_UNIT 
#define BUFFER_UNIT 4096
#endif 

char* utf8_to_ansi(char* str)
{
    wchar_t* uni = utf8_to_unicode(str);
    char* ansi = unicode_to_ansi(uni);
    free(uni);
    return ansi;
}

char* ansi_to_utf8(char* str)
{
    wchar_t* uni = ansi_to_unicode(str);
    char* utf8 = unicode_to_utf8(uni);
    free(uni);
    return utf8;
}

int file_exist(const char *file_name)
{
    FILE* fp = NULL;
    fp = fopen(file_name, "rb");
    if (fp)
    {
        fclose(fp);
    }
    return fp != NULL;
}

int remove_file(char *file_name)
{
#ifdef WIN32
    return DeleteFileA(file_name);
#else
	return remove(file_name);
#endif 
}

int  file_len(const char* file_name) {
	FILE* fp = NULL;
	fp = fopen(file_name, "rb");
	if (NULL == fp)
		return 0;//
	
	fseek(fp, 0, SEEK_END);

	int nLen = ftell(fp);

	fclose(fp);
	return nLen;
}

char* file_ext(char* file_name)
{
    char *ext = NULL;
    int i;

    for (i = strlen(file_name) - 1; i >= 0; i--)
    {
        if (file_name[i] == '.')
        {
            ext = file_name + i + 1;
            break;
        }
    }
    return ext;
}

static char logdir[MAX_PATH] = { 0 };
static char root[MAX_PATH] = { 0 };

void set_root_path(const char* path) {
	if (NULL != path) {
		int nLen = strlen(path);
		nLen = nLen > MAX_PATH - 1 ? MAX_PATH - 1 : nLen;
		memcpy(root, path, nLen);
	}
}

void set_log_path(const char* path) {
	if (NULL != path) {
		int nLen = strlen(path);
		nLen = nLen > MAX_PATH - 1 ? MAX_PATH - 1 : nLen;
		memcpy(logdir, path, nLen);
	}
}

static void  get_pwd_path(char* path , int nLen) {
	int i = 0;
	char szPath[MAX_PATH] = { 0 };
#ifdef WIN32
	GetCurrentDirectoryA(MAX_PATH, szPath);
#else
	char *p = getcwd(szPath, MAX_PATH);
#endif
	for (i = 0; i < strlen(szPath); i++)
	{
		if (szPath[i] == '\\')
		{
			szPath[i] = '/';
		}
	}
	if (szPath[strlen(szPath) - 1] != '/')
	{
		szPath[strlen(szPath)] = '/';
	}

	if (nLen > strlen(szPath) + 1)
		nLen = strlen(szPath) + 1;

	memcpy(path, szPath, nLen);
}

char* root_path()
{

    if (!root[0])
    {
		get_pwd_path(root, MAX_PATH - 1);
    }
    return root;
}

char* log_path()
{

	if (!logdir[0])
	{
		get_pwd_path(logdir, MAX_PATH - 1);
	}
	return logdir;
}
char* uint32_to_str(unsigned int n)
{
    static char buf[16] = {0};
    memset(buf, 0, sizeof(buf));
#ifdef WIN32
    _itoa(n, buf, 10);
#else
	sprintf(buf ,"%d",n);
#endif
	return buf;
}

#ifndef WIN32

int readFileList(char *basePath)
{
	DIR *dir = NULL;
	struct dirent *ptr = {0};
	char base[MAX_PATH] = {0};

	if ((dir = opendir(basePath)) == NULL)
	{
		fprintf(stderr ,"Open dir error...");
		return -1;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8)    ///file
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		else if (ptr->d_type == 10)    ///link file
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		else if (ptr->d_type == 4)    ///dir
		{
			memset(base, '\0', sizeof(base));
			strcpy(base, basePath);
			strcat(base, "/");
			strcat(base, ptr->d_name);
			readFileList(base);
		}
	}
	closedir(dir);
	return 1;
}

char* local_file_list(char *basePath)
{
	DIR *dir = NULL;
	struct dirent *ptr = { 0 };
	char base[MAX_PATH] = { 0 };

	const char* format_dir = "<a href=\"%s/\">%s/</a>" CRLF;
	const char* format_file = "<a href=\"%s\">%s</a>";

	char *result = NULL;
	char line[BUFFER_UNIT] = { 0 };
	int line_length;
	int size = BUFFER_UNIT;
	int offset = 0;
	char *size_str = NULL;
	char *utf8 = NULL;
	int i = 0;

	if ((dir = opendir(basePath)) == NULL)
	{
		log_error("[%s:%s:%d] Open dir error %s...",
			__FILE__,__FUNCTION__,__LINE__,basePath);
		return NULL;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8  || ptr->d_type == 10 )   ///file  or link
		{
			if (NULL == result)
				result = (char*)malloc(size);
			
			utf8 = ansi_to_utf8(ptr->d_name);
			sprintf(line, format_file, utf8, utf8);
			line_length = strlen(line);
			for (i = strlen(ptr->d_name); i < 60; i++)
			{
				line[line_length++] = ' ';
			}
			
			size_str = ""; // size of the file 
			//size_str = uint32_to_str(FindFileData.nFileSizeLow);

			memcpy(line + line_length, size_str, strlen(size_str));
			line_length += strlen(size_str);
			line[line_length++] = CR;
			line[line_length++] = LF;
			line[line_length] = 0;

			if (offset + line_length > size - 1)
			{
				size += BUFFER_UNIT;
				result = (char*)realloc(result, size);
			}
			if (NULL == result) {
				free(utf8);
				return NULL;
			}
				
			memcpy(result + offset, line, line_length);
			offset += line_length;
			free(utf8);
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		}
		else if (ptr->d_type == 4)    ///dir
		{
			if (NULL == result)
				result = (char*)malloc(size);
			
			utf8 = ansi_to_utf8(ptr->d_name);
			sprintf(line, format_dir, utf8, utf8);
			line_length = strlen(line);
			line[line_length] = 0;
			if (offset + line_length > size - 1)
			{
				size += BUFFER_UNIT;
				result = (char*)realloc(result, size);
			}
			if (NULL == result) {
				free(utf8);
				return NULL;
			}
				
			memcpy(result + offset, line, line_length);
			offset += line_length;
			free(utf8);
		}
	}
	closedir(dir);

	if(result)
		result[offset] = 0;

	return result;
}



int WSAGetLastError() {
	return errno;
}

/*
	0  ±íÊ¾´æÔÚ
*/
int dir_exist(char *file_name) {
	return access(file_name, 0);
}

void mkdirs(char *muldir)
{
	int i = 0, len = 0;
	char str[MAX_PATH] = { 0 };
	strncpy(str, muldir, MAX_PATH - 1);
	len = strlen(str);
	for (i = 0; i < len; i++)
	{
		if (str[i] == '/' || str[i] == '\\' && i != 0)
		{
			str[i] = '\0';
			if (access(str, 0) != 0)
			{
				mkdir(str, 0777);

			}
			str[i] = '/';
		}
	}
	if (len > 0 && access(str, 0) != 0)
	{
		mkdir(str, 0777);
	}
	return;
}

#else
/*
	0  ±íÊ¾´æÔÚ
*/
int dir_exist(char* file_name) {
	WIN32_FIND_DATA  FindFileData;
	BOOL bValue = FALSE;
	HANDLE hFind = FindFirstFile(file_name, &FindFileData);
	if ((hFind != INVALID_HANDLE_VALUE) && 
		(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		bValue = TRUE;
	}
	FindClose(hFind);
	return !bValue;// 0 ±íÊ¾´æÔÚ
}

void mkdirs(char *muldir)
{
	int i = 0, len = 0;
	char str[MAX_PATH] = { 0 };
	strncpy(str, muldir, MAX_PATH - 1);
	len = strlen(str);
	for (i = 0; i < len; i++)
	{
		if (str[i] == '/' || str[i] == '\\' && i != 0)
		{
			str[i] = '\0';
			if (_access(str, 0) != 0)
			{
				_mkdir(str);
			}
			str[i] = '/';
		}
	}
	if (len > 0 && _access(str, 0) != 0)
	{
		_mkdir(str);
	}
	return;
}

char* local_file_list(char *path)
{
	const char* format_dir = "<a href=\"%s/\">%s/</a>" CRLF;
	const char* format_file = "<a href=\"%s\">%s</a>";

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = NULL;

	char filter[MAX_PATH] = { 0 };
	char *result = NULL;
	char line[BUFFER_UNIT] = { 0 };
	int line_length;
	int size = BUFFER_UNIT;
	int offset = 0;
	char *size_str = NULL;
	char *utf8 = NULL;
	int i;

	sprintf(filter, "%s*", path);
	// list directory

	hFind = FindFirstFileA(filter, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		//log_error("{%s:%d} Invalid File Handle. GetLastError=%d", __FUNCTION__, __LINE__, GetLastError());
		return NULL;
	}
	do
	{
		if (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
		{
			if (!result)
			{
				result = (char*)malloc(size);
			}
			utf8 = ansi_to_utf8(FindFileData.cFileName);
			sprintf(line, format_dir, utf8, utf8);
			line_length = strlen(line);
			line[line_length] = 0;
			if (offset + line_length > size - 1)
			{
				size += BUFFER_UNIT;
				result = (char*)realloc(result, size);
			}

			if (NULL == result) {
				free(utf8);
				return NULL;
			}
			memcpy(result + offset, line, line_length);
			offset += line_length;
			free(utf8);
		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);

	// list files
	hFind = FindFirstFileA(filter, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		//log_error("{%s:%d} Invalid File Handle. GetLastError=%d", __FUNCTION__, __LINE__, GetLastError());
		return NULL;
	}
	do
	{
		if (!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes))
		{
			if (!result)
			{
				result = (char*)malloc(size);
			}
			utf8 = ansi_to_utf8(FindFileData.cFileName);
			sprintf(line, format_file, utf8, utf8);
			line_length = strlen(line);
			for (i = strlen(FindFileData.cFileName); i < 60; i++)
			{
				line[line_length++] = ' ';
			}
			size_str = uint32_to_str(FindFileData.nFileSizeLow);
			memcpy(line + line_length, size_str, strlen(size_str));
			line_length += strlen(size_str);
			line[line_length++] = CR;
			line[line_length++] = LF;
			line[line_length] = 0;

			if (offset + line_length > size - 1)
			{
				size += BUFFER_UNIT;
				result = (char*)realloc(result, size);
			}
			if (NULL == result) {
				free(utf8);
				return NULL;
			}
			memcpy(result + offset, line, line_length);
			offset += line_length;
			free(utf8);
		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);
	
	if(result)
		result[offset] = 0;

	return result;
}




#endif

static char sDefaultPath[MAX_PATH] = {0};
#define DEFAULT_STORE_ID  "Default"
char* get_default_store_path() {
	if (strlen(sDefaultPath) == 0) {
		strcpy(sDefaultPath , DEFAULT_STORE_ID);
	}

	return sDefaultPath;
}

void set_default_store_path(const char* path) {
	int nLen = strlen(path);
	if (nLen > MAX_PATH - 1)
		nLen = MAX_PATH - 1;

	memset(sDefaultPath, 0x00, sizeof(sDefaultPath));
	memcpy(sDefaultPath, path, nLen);
}







