#ifdef WIN32
#include <Winsock2.h>
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "utils.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

#ifndef WIN32
int local_file_list(const char *path, std::string* outfiles, int maxNo) {
	DIR *dir = NULL;
	struct dirent *ptr = { 0 };

	char *utf8 = NULL;
	int iFiles = 0;

	if ((dir = opendir(path)) == NULL)
	{
		fprintf(stderr, "Open dir error...");
		return 0;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8 || ptr->d_type == 10)   ///file  or link
		{
			if (iFiles >= maxNo) {
				break;
			}
			utf8 = ansi_to_utf8(ptr->d_name);
			std::string sFullPath(path);
			sFullPath += utf8;
			outfiles[iFiles++].assign(sFullPath);
			free(utf8);
			printf("d_name:%s%s\n", path, ptr->d_name);
		}
		else if (ptr->d_type == 4)    ///dir
		{

		}
	}
	closedir(dir);

	return iFiles;
}
static int customFilter(const struct dirent *pDir)
{
	if (pDir && pDir->d_type & 0x08) {
		char* pFound1 = strstr((char*)pDir->d_name, "_t_");
		char* pFound2 = strstr((char*)pDir->d_name, "_i_");

		if (NULL != pFound1 && NULL != pFound2)
			return 1;
	}
	else if (pDir && pDir->d_type & 0x04  
		&& strcmp(pDir->d_name , ".") 
		&& strcmp(pDir->d_name , ".."))
	{
		//dir
		return 1;
	}

	return 0;
}

int local_file_list1(const char *path, std::string* outfiles,int nStart, int maxN)
{
	struct dirent **namelist = NULL;
	int n = 0;
	int i = 0;
	int iFiles = nStart;
	n = scandir(path, &namelist, customFilter, alphasort);
	if (n <= 0)
	{
		log_info("{%s:%s:%d} local_file_list1  %s  not found file",
			__FILE__, __FUNCTION__, __LINE__, path);
		return 0;//
	}
	else
	{
		for (i = 0; i < n; i++)
		{
			//printf("%s\n", namelist[i]->d_name);
			if (namelist[i]->d_type & 0x08) {
				if (iFiles < maxN) {
					std::string fileName(path);
					fileName += namelist[i]->d_name;//
					outfiles[iFiles++] = fileName;
				}
			}
			else if(namelist[i]->d_type & 0x04) {
				std::string dirName(path);
				dirName += namelist[i]->d_name;
				dirName += "/";
				int nFileAdd = local_file_list1(dirName.c_str(), outfiles , iFiles, maxN);
				iFiles += nFileAdd;
			}
			
			free(namelist[i]);
		}
		free(namelist);
	}
	log_info("{%s:%s:%d} local_file_list1  %s  found files %d",
		__FILE__, __FUNCTION__, __LINE__, path , iFiles);
	return iFiles;
}


#else 
#define BUFFER_UNIT 512

int local_file_list(const char *path, std::string* outfiles, int maxNo)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = NULL;

	char filter[MAX_PATH] = { 0 };

	char *utf8 = NULL;
	int iFiles = 0;

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
			//
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
			if (iFiles >= maxNo) {
				break;
			}

			utf8 = ansi_to_utf8(FindFileData.cFileName);
			std::string sFullPath(path);
			sFullPath += utf8;

			outfiles[iFiles++].assign(sFullPath);
			free(utf8);
		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);

	return iFiles;
}

int local_file_list1(const char *path, std::string* outfiles , int nStart, int maxNo) {
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = NULL;

	char filter[MAX_PATH] = { 0 };

	char *utf8 = NULL;
	int iFiles = 0;

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
			//
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
			if (iFiles >= maxNo) {
				break;
			}

			utf8 = ansi_to_utf8(FindFileData.cFileName);
			std::string sFullPath(path);
			sFullPath += utf8;

			outfiles[iFiles++].assign(sFullPath);
			free(utf8);
		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);

	return iFiles;
}
#endif