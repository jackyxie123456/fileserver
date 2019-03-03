#pragma once

#include "TaskQueue.h"
#include "DbHelper.h"
#include "logger.h"

class DbTask : public Task {
public:
	DbTask(DbHelper& dbHelper , FileDbItem& fileItem):
		m_dbHelper(dbHelper){
		m_fileItem = fileItem;
	}

	virtual ~DbTask(){
	}

	/*
		0  Success
		other  failed
	*/
	virtual int Run();

	virtual std::string taskDescr() const {
		std::string taskDesc("");
		taskDesc += m_fileItem.szFileName;
		taskDesc += " DbTask Insert Or Update ";
		return taskDesc;
	}

	virtual void FinishCb() {
		
		log_info("{%s:%s:%d} DBTask %s Finish @%s  ",
			__FILE__, __FUNCTION__, __LINE__, taskDescr().c_str(),
			getCurTime().c_str()) ;
		delete this;
	}

protected:
	FileDbItem m_fileItem;
	DbHelper&  m_dbHelper;
};

