#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
#endif

wchar_t* ansi_to_unicode(char* str)
{
	wchar_t* result;
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	result = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	memset(result, 0, (len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)result, len);
	return result;
}

char* unicode_to_ansi(wchar_t* str)
{
	char* result;
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char*)malloc((len + 1) * sizeof(char));
	memset(result, 0, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, len, NULL, NULL);
	return result;
}

wchar_t* utf8_to_unicode(char* str)
{
	wchar_t* result;
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	memset(result, 0, (len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, len);
	return result;
}

char* unicode_to_utf8(wchar_t* str)
{
	char* result;
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char*)malloc((len + 1) * sizeof(char));
	memset(result, 0, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, len, NULL, NULL);
	return result;
}