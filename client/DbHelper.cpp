#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "DbHelper.h"

#define TABLESQL "create table if not exists %s (" \
				"id INTEGER PRIMARY KEY AUTOINCREMENT ,"  \
				"file_name text not null," \
				"create_time int null ," \
			    "file_size int null ," \
				"file_idx int null ," \
				"file_total int null," \
				"file_hash text null," \
				"is_upload int not null "\
				");"



int sqlite3_callback_func(void* pHandle, 
						int iRet,
						char** szSrc,
						char** szDst)
{
	//... 
	if (1 == iRet)
	{
		int iTableExist = atoi(*(szSrc));  //此处返回值为查询到同名表的个数，没有则为0，否则大于0
		if (pHandle != nullptr)
		{
			int* pRes = (int*)pHandle;
			*pRes = iTableExist;
		}
		// szDst 指向的内容为"count(*)"
	}

	return 0; //返回值一定要写，否则下次调用 sqlite3_exec(...) 时会返回 SQLITE_ABORT
}

/* callback函数只有在对数据库进行select, 操作时才会调用 */
static int select_callback(void *data, int argc, char **argv, char **azColName) {
	int i = 0;

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] == NULL? argv[i] : "NULL");
	}
	//printf("\n");
	return 0;
}


bool DbHelper::IsTableExist(const std::string& strTableName)
{
	std::string strFindTable =
		"SELECT COUNT(*) FROM sqlite_master where type ='table' and name ='" + strTableName + "'";

	char* sErrMsg = nullptr;

	//void *pHandle = ***;
	int nTableNums = 0;
	if (sqlite3_exec(m_db,
		strFindTable.c_str(),
		&sqlite3_callback_func, &nTableNums, &sErrMsg) != SQLITE_OK)
	{
		return false;
	}
	//回调函数无返回值，则此处第一次时返回SQLITE_OK， 第n次会返回SQLITE_ABORT

	return nTableNums > 0;
}

bool DbHelper::open() {

	if (nullptr != m_db)
		return true;

	/* 数据库创建或打开 */
	int rc = sqlite3_open(m_sDbName.c_str(), &m_db);

	if (rc) {
		
		log_error("{%s:%s:%d} Can't open database: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, sqlite3_errmsg(m_db));

		return false;
	}
	else {

		log_info("{%s:%s:%d} Opened database successfully\n ",
			__FILE__, __FUNCTION__, __LINE__);
	}
	return true;
}

void DbHelper::close() {
	if (nullptr == m_db)
		return;

	sqlite3_close(m_db);
	m_db = nullptr;
}


int DbHelper::createTable() {

	char sql5[512] = {0};
	char* tname = TABLE_NAME;
	char* szErrMsg = NULL;
	sprintf(sql5, TABLESQL, tname);

	printf("%s\n", sql5);
	open();
	/* 创建表 */
	int rc = sqlite3_exec(m_db, sql5, NULL, NULL, &szErrMsg);
	if (rc != SQLITE_OK) {
		
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg);

		sqlite3_free(szErrMsg);
	}
	else {
		log_info("{%s:%s:%d} Table created successfully\n ",
			__FILE__, __FUNCTION__, __LINE__);
	}

	return rc;
}

int DbHelper::createIndex() {
	return _createIndex("file_name");
}

int DbHelper::_createIndex(char* colName) {

	char sql5[512] = { 0 };

	char* szErrMsg = NULL;
	char szIdx[100] = { 0 };
	sprintf(szIdx ,"idx_%s",colName);
	sprintf(sql5, "create index if not exists %s on %s(%s)", 
					szIdx, TABLE_NAME, colName);

	printf("%s\n", sql5);
	open();
	/* 创建表 */
	int rc = sqlite3_exec(m_db, sql5, NULL, NULL, &szErrMsg);
	if (rc != SQLITE_OK) {
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg);

		sqlite3_free(szErrMsg);
	}
	else {
		log_info("{%s:%s:%d}Index created successfully\n",
			__FILE__, __FUNCTION__, __LINE__);
	}

	return rc;
}


int DbHelper::InsertOne(FileDbItem* fileItem) {

	
	char* tableName = TABLE_NAME;
	char sql2[512] = { 0 };
	/* 不推荐使用这种方式 */
	sprintf(sql2, 
		"insert into %s (file_name, create_time, is_upload,file_size,file_idx,file_total,file_hash) " 
		"values ('%s', '%d', '%d' ,'%d','%d','%d','%s');",
		tableName,
		fileItem->szFileName,
		fileItem->nCreateTime,
		fileItem->isUpload,
		fileItem->nSize,
		fileItem->nIndex,
		fileItem->nTotal,
		fileItem->szHash
		);

	open();
	char* szErrMsg = NULL;
	/* 插入数据 */
	int rc = sqlite3_exec(m_db, sql2, NULL, NULL, &szErrMsg);
	if (rc != SQLITE_OK) {
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg); 
		sqlite3_free(szErrMsg);
	}
	else {
		fprintf(stdout, "Table insert data successfully\n");
	}
	return rc;
}

int DbHelper::UpdateItem(FileDbItem* fileItem) {
	char* data = "update call back function call!\n";
	/* update 使用*/
	char sql[512] = { 0 };

	char* sqlUpdate = "update %s set file_name = '%s' ,"
					" is_upload = '%d' , "
					" file_size = '%d' ,"
					" file_idx = '%d' , "
					" file_total = '%d' ,"
					" file_hash = '%s' "
					" where file_name = '%s' and file_idx = '%d' ;";

	char* szErrMsg = NULL;

	if (NULL == fileItem)
		return -1;

	open();

	sprintf(sql, sqlUpdate,
		TABLE_NAME,
		fileItem->szFileName,
		fileItem->isUpload,
		fileItem->nSize ,
		fileItem->nIndex ,
		fileItem->nTotal,
		fileItem->szHash,
		fileItem->szFileName,
		fileItem->nIndex);

	int rc = sqlite3_exec(m_db, sql, select_callback, data, &szErrMsg);
	if (rc != SQLITE_OK) {
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg);
		sqlite3_free(szErrMsg);
	}
	else {
		fprintf(stdout, "Table update successfully\n");
	}

	return rc;
}

int DbHelper::deleteTable()
{
	/* 删除表 */
	char sql[512] = { 0 };
	char* szErrMsg = NULL;
	sprintf(sql, "drop table %s;", TABLE_NAME);
	open();

	int rc = sqlite3_exec(m_db, sql, NULL, NULL, &szErrMsg);
	if (rc != SQLITE_OK) {
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg);
		sqlite3_free(szErrMsg);
	}
	else {
		fprintf(stdout, "Table droped successfully\n");
	}
	return rc;
}


int DbHelper::InsertAll(std::array<FileDbItem, 512>& items) {

	char sql[512] = { 0 };
	char* sqlInsert = "insert into %s (file_name, create_time, is_upload,file_size,file_idx,file_total,file_hash)"
		"values (:file_name, :create_time, :is_upload, :file_size,:file_idx , :file_total , :file_hash);";
	/* 注: ":sid" 为命名参数 也可以用 ? 号*/

	sprintf(sql, sqlInsert, TABLE_NAME);
	sqlite3_stmt *stmt = NULL;

	open();
	/* 准备一个语句对象 */
	sqlite3_prepare_v2(m_db, sql, strlen(sql), &stmt, NULL);

	/* 执行sql 语句对象并判断其返回值
		发现如果不是select 这样会产生结果集的操作
		返回值为SQLITE_DONE 或者出错，只有执行sql语句会产生
		结果集执行step函数才返回SQLITE_ROW*/
	int rc = 0;//

	int iItem = 0;
	for (iItem = 0; iItem < items.size(); iItem++) {

		FileDbItem fileItem = items[iItem];
		char * szFileName = fileItem.szFileName;
		char * szHash = fileItem.szHash;
		sqlite3_reset(stmt);  /* 如果要重新绑定其他值要reset一下 */
		sqlite3_bind_text(stmt, 1, szFileName, strlen(szFileName), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, fileItem.nCreateTime); /* 重新绑定值 */
		sqlite3_bind_int(stmt, 3, fileItem.isUpload);
		sqlite3_bind_int(stmt, 4, fileItem.nSize);
		sqlite3_bind_int(stmt, 5, fileItem.nIndex);
		sqlite3_bind_int(stmt, 6, fileItem.nTotal);
		sqlite3_bind_text(stmt, 7, szHash , strlen(szHash) , SQLITE_STATIC);

		rc =  sqlite3_step(stmt);  /* 再执行 */

		printf("step() return %s\n", rc == SQLITE_DONE ? "SQLITE_DONE"
			: (rc == SQLITE_ROW ? "SQLITE_ROW" : "SQLITE_ERROR"));

		//sqlite3_bind_blob(stmt, 1, sectionData, 4096, SQLITE_STATIC);
	}

   /* 销毁prepare 创建的语句对象 */
	sqlite3_finalize(stmt);

	return rc;
}

int DbHelper::DeleteItem(FileDbItem* fileItem) {

	char sql5[512] = { 0 };
	char* tname = "fileInfo";
	char* szErrMsg = NULL;
	sprintf(sql5, "create table if not exists %s ("\
		"id int not null," \
		"name text not null);", tname);

	//printf("%s\n", sql5);

	open();
	/* 创建表 */
	int rc = sqlite3_exec(m_db, sql5, NULL, NULL, &szErrMsg);
	if (rc != SQLITE_OK) {
		log_error("{%s:%s:%d} SQL error: %s\n ",
			__FILE__, __FUNCTION__, __LINE__, szErrMsg); 
		sqlite3_free(szErrMsg);
	}
	else {
		fprintf(stdout, "Delete  successfully\n");
	}

	return rc;
}

std::array<FileDbItem, 512> DbHelper::FindBy(
	std::map<std::string, std::string>& condition,
	int offset , int limits) {
	std::array<FileDbItem, 512> dbItems;
	
	/* 取数据 */
  //sql = "select * from healthinfo;";  
	char * sqlSelect = "select * from %s limit %d offset %d;"; 
	char sql[512] = {0};

	sprintf(sql, sqlSelect, TABLE_NAME, limits, offset);
	sqlite3_stmt *stmt = NULL;

	open();

	sqlite3_prepare_v2(m_db, sql, strlen(sql), &stmt, NULL);

	int iItem = 0;//	
	/* 遍历执行sql语句后的结果集的每一行数据 */
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		/* 获得字节数，第二个参数为select结果集合的列号 */
		/* 由于select 的结果集只有section这一列，因此为0 */
		int id = sqlite3_column_int(stmt, 0);
		const unsigned char* file_name = sqlite3_column_text(stmt, 1);
		int create_time = sqlite3_column_int(stmt, 2);
		int is_upload = sqlite3_column_int(stmt, 3);
		int file_size = sqlite3_column_int(stmt, 4);
		int file_idx = sqlite3_column_int(stmt, 5);
		int file_total = sqlite3_column_int(stmt, 6);
		const unsigned char*  file_hash = sqlite3_column_text(stmt, 7);

		FileDbItem fileItem;
		fileItem.isUpload = is_upload;
		fileItem.nCreateTime = create_time;
		fileItem.nSize = file_size;
		fileItem.nIndex = file_idx;
		fileItem.nTotal = file_total;


		strcpy(fileItem.szFileName, (const char*)file_name);
		strcpy(fileItem.szHash, (const char*)file_hash);


		dbItems[iItem++] = fileItem;
		//unsigned char* srcdata = sqlite3_column_blob(stmt, 0);  /* 取得数据库中的blob数据 */  
	}
	
	sqlite3_finalize(stmt);
	
	return dbItems;
}

int DbHelper::FindByName(std::string& fileName , 
							int index,
							FileDbItem* fileItem2) {

	
	char* sqlHead = "select * from %s where file_name = '%s' and file_idx = '%d' limit 1;";
	char sql[512] = { 0 };

	char* szErrMsg = NULL;
		
	sprintf(sql, sqlHead, TABLE_NAME, fileName.c_str());

	sqlite3_stmt *stmt = NULL;

	open();

	sqlite3_prepare_v2(m_db, sql, strlen(sql), &stmt, NULL);

	int iItem = 0;
	FileDbItem fileItem;
	/* 遍历执行sql语句后的结果集的每一行数据 */
	while (sqlite3_step(stmt) == SQLITE_ROW) {

		int id = sqlite3_column_int(stmt, 0);
		const unsigned char* file_name = sqlite3_column_text(stmt, 1);
		int create_time = sqlite3_column_int(stmt, 2);
		int is_upload = sqlite3_column_int(stmt, 3);
		int file_size = sqlite3_column_int(stmt, 4);
		int file_idx = sqlite3_column_int(stmt, 5);
		int file_total = sqlite3_column_int(stmt, 6);
		const unsigned char*  file_hash = sqlite3_column_text(stmt, 7);
		

		FileDbItem fileItem;
		fileItem.isUpload = is_upload;
		fileItem.nCreateTime = create_time;
		fileItem.nSize = file_size;
		fileItem.nIndex = file_idx;
		fileItem.nTotal = file_total;


		strcpy(fileItem.szFileName, (const char*)file_name);
		strcpy(fileItem.szHash, (const char*)file_hash);

		iItem++;
		//unsigned char* srcdata = sqlite3_column_blob(stmt, 0);  /* 取得数据库中的blob数据 */  
	}
	sqlite3_finalize(stmt);

	if (iItem > 0 && fileItem2)
		*fileItem2 = fileItem;
	return iItem;
}

DbHelper& GetDbHelper(const char* dbName) {
	static DbHelper dbHelper(dbName);

	return dbHelper;
}