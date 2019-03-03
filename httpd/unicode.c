#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#include <locale.h>
#endif

#ifdef WIN32

#include <Windows.h>


wchar_t* ansi_to_unicode(char* str)
{
	wchar_t* result;
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	result = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	if (NULL == result)
		return result;
	memset(result, 0, (len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)result, len);
	return result;
}

char* unicode_to_ansi(wchar_t* str)
{
	char* result;
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char*)malloc((len + 1) * sizeof(char));
	if (NULL == result)
		return result;
	memset(result, 0, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, len, NULL, NULL);
	return result;
}

wchar_t* utf8_to_unicode(char* str)
{
	wchar_t* result;
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	if (NULL == result)
		return result;
	memset(result, 0, (len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, len);
	return result;
}

char* unicode_to_utf8(wchar_t* str)
{
	char* result;
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char*)malloc((len + 1) * sizeof(char));
	if (NULL == result)
		return result;
	memset(result, 0, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, len, NULL, NULL);
	return result;
}



#else 

wchar_t* ansi_to_unicode(char* str)
{
	wchar_t * wsBuf = NULL;
	setlocale(LC_CTYPE, "");
	int dSize = mbstowcs(wsBuf, str, 0);
	
	wsBuf = malloc(sizeof(wchar_t) * (dSize + 1));
	if (NULL == wsBuf)
		return NULL;

	memset(wsBuf, 0, sizeof(wchar_t) * (dSize + 1));

	int nRet = mbstowcs(wsBuf, str, dSize);
	
	return  wsBuf;//
}

char* unicode_to_ansi(wchar_t* wsBuf)
{
	size_t sSize = wcslen(wsBuf);
	char * sBuf = NULL;

	setlocale(LC_CTYPE, "");

	int dSize = wcstombs(sBuf, wsBuf, 0);

	sBuf = malloc(dSize + 1);
	if (NULL == sBuf)
		return NULL;
	memset(sBuf, 0, dSize + 1);
	int nRet = wcstombs(sBuf, wsBuf, dSize);
	return sBuf;
}

unsigned char * make_utf8_string(const wchar_t *unicode)
{
	int size = 0, index = 0, out_index = 0;
	unsigned char *out;
	unsigned short c;

	/* first calculate the size of the target string */
	c = unicode[index++];
	while (c)
	{
		if (c < 0x0080)
		{
			size += 1;
		}
		else if (c < 0x0800)
		{
			size += 2;
		}
		else
		{
			size += 3;
		}

		c = unicode[index++];
	}

	out = (unsigned char*)malloc(size + 1);
	if (out == NULL)
		return NULL;

	index = 0;

	c = unicode[index++];
	while (c)
	{
		if (c < 0x080)
		{
			out[out_index++] = (unsigned char)c;
		}
		else if (c < 0x800)
		{
			out[out_index++] = 0xc0 | (c >> 6);
			out[out_index++] = 0x80 | (c & 0x3f);
		}
		else
		{
			out[out_index++] = 0xe0 | (c >> 12);
			out[out_index++] = 0x80 | ((c >> 6) & 0x3f);
			out[out_index++] = 0x80 | (c & 0x3f);
		}
		c = unicode[index++];
	}

	out[out_index] = 0x00;

	return out;
}

wchar_t * make_unicode_string(const unsigned char *utf8)
{
	int size = 0, index = 0, out_index = 0;
	wchar_t *out;
	unsigned char c;

	/* first calculate the size of the target string */
	c = utf8[index++];
	while (c)
	{
		if ((c & 0x80) == 0)
		{
			index += 0;
		}
		else if ((c & 0xe0) == 0xe0)
		{
			index += 2;
		}
		else
		{
			index += 1;
		}

		size += 1;
		c = utf8[index++];
	}

	out = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
	if (out == NULL)
		return NULL;

	index = 0;

	c = utf8[index++];
	while (c)
	{
		if ((c & 0x80) == 0)
		{
			out[out_index++] = c;
		}
		else if ((c & 0xe0) == 0xe0)
		{
			out[out_index] = (c & 0x1F) << 12;
			c = utf8[index++];
			out[out_index] |= (c & 0x3F) << 6;
			c = utf8[index++];
			out[out_index++] |= (c & 0x3F);
		}
		else
		{
			out[out_index] = (c & 0x3F) << 6;
			c = utf8[index++];
			out[out_index++] |= (c & 0x3F);
		}

		c = utf8[index++];
	}

	out[out_index] = 0;

	return out;
}

wchar_t* utf8_to_unicode(char* str)
{
	return make_unicode_string(str);
}

char* unicode_to_utf8(wchar_t* str)
{
	return make_utf8_string(str);
}


#endif
