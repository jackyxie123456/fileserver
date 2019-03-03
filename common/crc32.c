#include<stdio.h>
#include <malloc.h>

static unsigned int CRC32_table[256] = { 0 };
void init_CRC32_table()
{
	int i = 0;
	for ( i = 0; i != 256; i++)
	{
		unsigned int CRC = i;
		int j = 0;
		for ( j = 0; j != 8; j++)
		{
			if (CRC & 1)
				CRC = (CRC >> 1) ^ 0xEDB88320;
			else
				CRC >>= 1;
		}
		CRC32_table[i] = CRC;
	}
}

unsigned int crc32(unsigned char* buf, unsigned int len)
{
	unsigned int CRC32_data = 0xFFFFFFFF;
	unsigned int i = 0;
	for ( i = 0; i != len; ++i)
	{
		unsigned int t = (CRC32_data ^ buf[i]) & 0xFF;
		CRC32_data = ((CRC32_data >> 8) & 0xFFFFFF) ^ CRC32_table[t];
	}
	return ~CRC32_data;
}


/*计算buffer的crc校验码*/
unsigned int crc32_len(unsigned int crc, unsigned char *buf, unsigned int size)
{
	unsigned int CRC32_data = crc;
	unsigned int i = 0;
	for (i = 0; i < size; ++i)
	{
		unsigned int t = (CRC32_data ^ buf[i]) & 0xFF;
		CRC32_data = ((CRC32_data >> 8) & 0xFFFFFF) ^ CRC32_table[t];
	}
	return ~CRC32_data;
}

#define FILE_BUFF (4*1024*1024)

unsigned int calcCrcCode(const char* fileName ,int* nLen){

	FILE* pFile = fopen(fileName ,"rb+");
	if (NULL == pFile)
		return 0;
	char* pData = (char*)malloc(FILE_BUFF);

	if (NULL == pData)
	{
		fclose(pFile);
		return 0;
	}

	fseek(pFile,0 , SEEK_END);
	int nLen1 = ftell(pFile);
	fseek(pFile,0 , SEEK_SET);

	int nReadFinish = 0;
	unsigned int nCrc32 = 0xFFFFFFFF;
	while (nReadFinish < nLen1) {
		int nRead = fread(pData, 1, FILE_BUFF, pFile);

		nCrc32 = crc32_len(nCrc32, (unsigned char *)pData, nRead);

		nReadFinish += nRead;
	}

	fclose(pFile);
	free(pData);

	*nLen = nReadFinish;
	return nCrc32;
}


unsigned int  calcCrcCode2(const char* fileName,int* nLen) {

	FILE* pFile = fopen(fileName, "rb+");
	char* pData = NULL;

	if (NULL == pFile)
		return 0;

	fseek(pFile, 0, SEEK_END);
	int nLen1 = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	pData = (char*)malloc(nLen1);
	if (NULL == pData)
	{
		fclose(pFile);
		return 0;
	}
	memset(pData, 0x00, sizeof(nLen1));

	int nRead = fread(pData, 1, nLen1, pFile);

	unsigned int nCrcCode = crc32((unsigned char*)pData, nLen1);
	fclose(pFile);
	free(pData);

	*nLen = nRead;
	return nCrcCode;
}