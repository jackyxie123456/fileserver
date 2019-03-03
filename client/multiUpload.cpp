#include <stdio.h>  
#include <string.h>    
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include "utils.h"
#include "HttpParser.h"
#include "logger.h"
#include "crc32.h"

#ifdef WIN32
#include <conio.h>
#endif

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

#define UPLOAD_SUCC "Upload completed"

using namespace std;
//回调函数  得到响应内容
static int write_data(void* buffer, int size, int nmemb, void* userp) {
	std::string * str = dynamic_cast<std::string *>((std::string *)userp);
	str->append((char *)buffer, size * nmemb);

	return nmemb;
}

int  upload_part_data(std::string url,
					std::string fileName,
					std::string* response,
					int nTimeout,
					int nIndex,
					int nTotal, 
					const char* pData, 
					int nLen, 
					unsigned int nCrcCode);



int upload(std::string url,
			std::string fileName,
			std::string* response,
			int nTimeout)
{
	CURL *curl;
	CURLcode ret;
	curl = curl_easy_init();
	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;
	HttpParser httpParser;

	curl_slist *slist = NULL;
	slist = curl_slist_append(slist, "Accept: text/xml");
	slist = curl_slist_append(slist, "Depth: infinity");
	slist = curl_slist_append(slist, "Connection: Keep-Alive");
	slist = curl_slist_append(slist, "Content-Type: text/html");
	slist = curl_slist_append(slist, "Expect:");
	
	string sFileName("");
	int nLast = fileName.find_last_of("/\\");
	
	if (nLast == -1) {
		sFileName.assign(fileName);
	}
	else {
		sFileName.assign(fileName.substr(nLast + 1));
	}
	
	int nFileLen1 = 0;
	unsigned int nCrcCode = calcCrcCode(fileName.c_str() , &nFileLen1);
	int nLen = file_len(fileName.c_str());

	char szPathApp[MAX_PATH] = { 0 };
	sprintf(szPathApp, "index=%d&total=%d&crccode=%u&size=%d",0, 1, nCrcCode, nLen);

	url.append("&");
	url.append(szPathApp);

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, (char *)url.c_str());           //指定url

		curl_formadd(&post,
			&last,
			CURLFORM_PTRNAME, "file",
			CURLFORM_FILE, fileName.c_str(),
			CURLFORM_FILENAME, sFileName.c_str(),
			CURLFORM_END);// form-data key(file) "./test.jpg"为文件路径  "slamtv60.264" 为文件上传时文件名

		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);                     //构造post参数    
		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "8.8.8.8");

		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "114.114.114.114");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

	//	curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);//接收数据时超时设置，如果20秒内数据未接收完，直接退出

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);          //绑定相应
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);        //绑定响应内容的地址

		ret = curl_easy_perform(curl);                          //执行请求

		std::cout << response << std::endl;

		curl_easy_cleanup(curl);

		if (ret == CURLE_OK) {
			std::string titleContent = httpParser.parse(response->c_str());
			if (strncmp(UPLOAD_SUCC, titleContent.c_str(),
				strlen(UPLOAD_SUCC) ) == 0) {
				return 0;

			}
		}

		return ret;
	}
	else {
		return -1;
	}
}

/* from 0 ==> total - 1 */

int upload_part(std::string url,
				std::string fileName,
				std::string* response,
				int nPartSize,
				int nIndex,
				int nTimeout)
{
	FILE* pFile = fopen(fileName.c_str(), "rb+");
	if (NULL == pFile) {
		return -1;
	}
	int nLen = nPartSize;
	int nFileLen = file_len(fileName.c_str());
	int nTotal = 1;
	if (nFileLen%nPartSize == 0) {
		nTotal = nFileLen / nPartSize;
	}
	else {
		nTotal = nFileLen / nPartSize + 1;
	}

	if (nIndex == nTotal - 1) {
		// 最后一块
		nLen = nFileLen % nPartSize;
	}

	int nSeek = nIndex * nPartSize;
	char* pData = (char*)malloc(nLen);

	if (NULL == pData) {
		fclose(pFile);
		return -2;
	}
	memset(pData, 0x00, sizeof(nLen));

	fseek(pFile, nSeek, SEEK_SET);
	int nRead = fread(pData, 1, nLen, pFile);

	unsigned int nCrcCode = crc32((unsigned char*)pData , nLen);
	log_info("{%s:%s:%d} upload_part %s ,index: %d , total: %d ,seek %d ,nLen:%d ,nRead %d \r\n",
		__FILE__, __FUNCTION__, __LINE__,
		fileName.c_str() , nIndex , nTotal ,
		nSeek , nLen , nRead);

	int nUpload = upload_part_data(url, fileName,
									response, nTimeout,
									nIndex, nTotal,
									pData, nLen , nCrcCode);
	
	fclose(pFile);
	free(pData);
	return nUpload;
}

unsigned int  writeUpload_part(std::string fileName, 
					  int nIndex ,
					  int nTotal , const char* pData , int nLen ) {
	char szPathApp[MAX_PATH] = { 0 };
	sprintf(szPathApp, "_t_%d_i_%d",
		nTotal, nIndex);
	
	fileName.append(szPathApp);

	FILE* pFile = fopen(fileName.c_str(), "wb+");
	if (NULL == pFile) {
		return 0;
	}

	fwrite(pData, 1, nLen, pFile);

	fclose(pFile);

	int nFileLen = 0;
	unsigned int nCrc = calcCrcCode(fileName.c_str(), &nFileLen);

	return nCrc;
}

#define META_POST ".meta"

static int save_meta_data(const char* fileName, int nIndex, int nTotal, int nSize, unsigned int nCrccode) {
	char szPath[MAX_PATH] = { 0 };
	FILE* pFile = NULL;

	char* pTotal = strstr((char*)fileName, "_t_");
	if (NULL == pTotal)
	{
		memcpy(szPath, fileName, strlen(fileName));
	}
	else {
		memcpy(szPath, fileName, pTotal - fileName);
	}

	int nLenCat = strlen(META_POST);
	nLenCat = nLenCat > (MAX_PATH - strlen(szPath)) ? (MAX_PATH - strlen(szPath)) : nLenCat;
	strncat(szPath, META_POST, nLenCat);
	pFile = fopen(szPath, "rb+");
	if (NULL == pFile)
		return -1;

	fseek(pFile, 0, SEEK_END);

	char szBuff[1024] = { 0 };

	sprintf(szBuff, "%d,%s,%d,%d,%u\r\n",
		nIndex, fileName, nTotal, nSize, nCrccode);
	fputs(szBuff, pFile);
	fclose(pFile);

	return 0;
}

int  upload_part_data(std::string url,
					  std::string fileName,
					  std::string* response,
					  int nTimeout,
					  int nIndex,
					  int nTotal,
					  const char* pData , 
					  int nLen,
					  unsigned int nCrcCode) {
	CURL *curl;
	CURLcode ret;
	curl = curl_easy_init();
	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;
	HttpParser httpParser;

	curl_slist *slist = NULL;
	slist = curl_slist_append(slist, "Accept: text/xml");
	slist = curl_slist_append(slist, "Depth: infinity");
	slist = curl_slist_append(slist, "Connection: Keep-Alive");
	//slist = curl_slist_append(slist, "Content-Type: text/html");
	slist = curl_slist_append(slist, "Expect:");

	string sFileName("");
	int nLast = fileName.find_last_of("/\\");
	
	if (nLast == -1) {
		sFileName.assign(fileName);
	}
	else {
		sFileName.assign(fileName.substr(nLast + 1));
	}

	char szPathApp[MAX_PATH] = { 0 };
	sprintf(szPathApp, "index=%d&total=%d&crccode=%u&size=%d",
		nIndex, nTotal,nCrcCode,nLen);

	url.append("&");
	url.append(szPathApp);

	int nTimeStart = time(0);
	int nTimeEnd = 0;//

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, (char *)url.c_str());           //指定url

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


		curl_formadd(&post, &last,
			CURLFORM_COPYNAME, "filename",
			CURLFORM_BUFFER, sFileName.c_str(),
			CURLFORM_BUFFERPTR, pData,
			CURLFORM_BUFFERLENGTH, nLen,
			CURLFORM_CONTENTTYPE, "application/octet-stream",
			CURLFORM_END);

		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "8.8.8.8");

		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "114.114.114.114");

		curl_easy_setopt(curl , CURLOPT_BUFFERSIZE , 65536);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);                     //构造post参数    

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);//接收数据时超时设置，如果20秒内数据未接收完，直接退出
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);          //绑定相应
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);        //绑定响应内容的地址

		ret = curl_easy_perform(curl);                          //执行请求

		nTimeEnd = time(0);

		std::cout << response << std::endl;

		curl_easy_cleanup(curl);

		unsigned int nCrc = writeUpload_part(fileName, nIndex, nTotal, pData, nLen);

		save_meta_data(fileName.c_str(), nIndex, nTotal,nLen ,nCrc);

		log_info("upload %s nCrc %d nCrcFile %d\r\n",
			fileName.c_str(), nCrcCode , nCrc);

		if (ret == CURLE_OK) {
			std::string titleContent = httpParser.parse(response->c_str());
			if (strncmp(UPLOAD_SUCC, titleContent.c_str(),
				strlen(UPLOAD_SUCC)) == 0) {
				
				log_info("upload %s succ cost %d \r\n",
					fileName.c_str(), nTimeEnd - nTimeStart);
				return 0;
			}
		}

		return ret == 0 ? -2 : ret ;
	}
	else {
		return -1;
	}
}


void MSleep(int nMs) {
#ifdef WIN32
	Sleep(nMs);
#else
	usleep(nMs * 1000);
#endif
}


int  checkBreak() {
#ifdef WIN32
	if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真
		int ch = _getch();//使用_getch()函数获取按下的键值

		if (ch == 27) {
			std::cout << "ESC Press Quit " << std::endl;
			return 1;
		}//当按下ESC时循环，ESC键的键值时27.
	}
#endif

	return 0;//
}
