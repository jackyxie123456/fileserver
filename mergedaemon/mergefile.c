#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

#define MERGE_BUF_SZ (4 * 1024 * 1024)

int  mergefile1(const char* fileIn  ,FILE* pOut) {
	
	FILE* pFileIn = NULL;
	pFileIn = fopen(fileIn, "rb+");

	log_info("{%s:%s:%d} mergefile1 %s %x ",
		__FILE__, __FUNCTION__, __LINE__,
		fileIn , pOut);

	if (NULL == pFileIn )
		return -1;

	char* szBuff = (char*)malloc(MERGE_BUF_SZ);
	if (NULL == szBuff)
	{
		fclose(pFileIn);
		return -2;
	}

	fseek(pFileIn, 0, SEEK_END);

	int nLen = ftell(pFileIn);

	int nFinish = 0;
	fseek(pFileIn, 0, SEEK_SET);

	while (nFinish < nLen) {

		int nLeft = MERGE_BUF_SZ;
		nLeft = (nLeft > (nLen - nFinish)) ? (nLen - nFinish) : nLeft;

		int nRead = fread(szBuff, 1, nLeft , pFileIn);

		
		if (nRead > 0) {
			fwrite(szBuff, 1, nRead, pOut);
			fflush(pOut);
		}
		else {
			log_error("{%s:%s:%d} mergefile1 %s read  %d ret %d ",
				__FILE__, __FUNCTION__, __LINE__,
				fileIn, nLeft, nRead);
		}

		nFinish += nRead;
	}

	free(szBuff);
	fclose(pFileIn);
	return nFinish;
}


int  mergefile(const char** fileIn,
				int nFiles,
				const char* szOut) {

	FILE * pFileOut = NULL;
	pFileOut = fopen(szOut, "ab+");

	if (NULL == pFileOut)
		return -1;
	int nResult = 0;
	int iFile = 0;
	for (iFile = 0; iFile < nFiles; iFile++) {
		nResult = mergefile1(fileIn[iFile], pFileOut);
		if (nResult < 0) {
			fclose(pFileOut);
			log_error("{%s:%s:%d} merge failed %s %d ==> %s ",
				__FILE__, __FUNCTION__, __LINE__,
				fileIn[iFile], iFile, szOut);
				
			return nResult;
		}
	}

	fclose(pFileOut);
	log_info("{%s:%s:%d} merge  %d ==> %s suc!!",
		__FILE__, __FUNCTION__, __LINE__, nFiles , szOut);
	return iFile;

}


