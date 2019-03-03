#pragma once

#include <string>
#include <array>
#include <sqlite3.h>

#include <map>
#include <string>
#include <memory.h>
#include <stdlib.h>

#ifndef MAX_PATH 
#define MAX_PATH 260
#endif

#define  HASH_SIZE 128

 struct FileDbItem {
	char szFileName[MAX_PATH]; /*fileName on disk */
	int  isUpload;      /* isUpload Or Not */
	int  nCreateTime;  /* create time  long */
	int  nTotal;// if not segment  nTotal == 1
	int  nIndex;// if not segment  nIndex == 0
	int  nSize;// filesize
	char szHash[HASH_SIZE];//
 public:
	FileDbItem():
		isUpload(0),
		nCreateTime(0)
	{
		memset(szFileName, 0x00, sizeof(szFileName));
		memset(szHash, 0x00, sizeof(szHash));
	}

	FileDbItem(const FileDbItem& left) {
		this->operator=(left);
	}
	void operator = (const FileDbItem& left) {
		isUpload = left.isUpload;
		nCreateTime = left.nCreateTime;
		nIndex = left.nIndex;
		nTotal = left.nTotal;
		nSize = left.nSize;

		memset(szFileName, 0x00, sizeof(szFileName));
		memcpy(szFileName, left.szFileName, sizeof(szFileName));

		memset(szHash, 0x00, sizeof(szHash));
		memcpy(szHash, left.szHash, sizeof(szHash));
	}
};

inline void copyTo(const FileDbItem* src, FileDbItem* dest) {
	memcpy(dest, src, sizeof(src));
}

#define TABLE_NAME "fileInfo"
#define DB_NAME  "luckin_iot.db"

class DbHelper {
public:
	DbHelper(const char* dbName = DB_NAME)
		:m_db(nullptr),
		m_sDbName(dbName)
	{
		
	}
 
	virtual ~DbHelper() {
		close();
	}

	void SetDbName(const char* szName) {
		close();
		m_sDbName.assign(szName);
	}

	bool IsTableExist(const std::string& strTableName);

	int createTable();

	int deleteTable();

	int createIndex();

	int InsertOne(FileDbItem* fileItem);

	int UpdateItem(FileDbItem* fileItem);

	int InsertAll(std::array<FileDbItem , 512>& items);

	int DeleteItem(FileDbItem* fileItem);

	/*
		1  found
		0  not found 
	*/
	int FindByName(std::string& fileName,int index  ,FileDbItem* fileItem2);
	
	std::array<FileDbItem, 512> FindBy(
		std::map<std::string , std::string>& condition,
		int offset, int limits);

	
protected:
	bool open();

	void close();

	int _createIndex(char* colName);
protected:
	DbHelper(const DbHelper&){}

	sqlite3 *m_db;

	std::string m_sDbName;
};

DbHelper& GetDbHelper(const char* dbName = DB_NAME);
