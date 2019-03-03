#pragma once
#include <string>
#include "utils.h"
#include "pthread.h"

#define EVENT_FILE_CREATE 0x00
#define EVENT_DIR_CREATE  0x01
#define EVENT_FILE_DELETE 0x02
#define EVENT_DIR_DELETE  0x03

typedef struct __FileEvent{
	char szFileName[MAX_PATH];
	int  szEventType;
}FileEvent;

typedef void (*FileEvCb)(FileEvent* , void* ) ;

class FileWatch{
public:
	FileWatch(const char* szPath = ""):
		m_nNotifyFd(-1),
		m_pCb(NULL),
		m_bExit(0),
		m_sPath(szPath),
		m_pObject(NULL)
	{

	}
	virtual ~FileWatch() {
		Stop();
	}

	void Start();

	void Stop();

	void SetEvCb(FileEvCb pCb , void* pObject) {
		m_pCb = pCb;
		m_pObject = pObject;
	}
	
	void DoWork();
protected:
	FileWatch(const FileWatch& ){}

	int stopWatch(int iNotifyFd);
	void startWatch(const char* szPath);
protected:
	std::string m_sPath;
	int  m_nNotifyFd;


	int  watch_inotify_events(int fd);
	void readEvent(void*);

protected:
	FileEvCb m_pCb;
	void*  m_pObject;

	int  m_bExit;
	pthread_t m_watch_t;
};



