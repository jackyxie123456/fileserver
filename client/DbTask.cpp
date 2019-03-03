#include "DbTask.h"
#include "DbHelper.h"


int DbTask::Run() {

	int nFound = 0;
	int rc = 0;
	int index = m_fileItem.nIndex;
	std::string fileName(m_fileItem.szFileName);

	log_info("{%s:%s:%d} DbTask  %s  Running @ %s ",
		__FILE__, __FUNCTION__, __LINE__, taskDescr().c_str(),
		getCurTime().c_str());

	if (fileName.length() == 0) {
		return 0;//
	}

	FileDbItem fileDbItem;
		
	if (m_dbHelper.FindByName(fileName ,index , &fileDbItem) == 0) {
		rc = m_dbHelper.InsertOne(&m_fileItem);
	}
	else {
		rc = m_dbHelper.UpdateItem(&m_fileItem);
	}

	return rc;
}