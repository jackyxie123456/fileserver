#ifndef WIN32
#include <sys/inotify.h>  
#include <unistd.h>
#endif
#include <string.h>  
#include <stdio.h>  
#include <iostream>

#include "watch.h"
#include "utils.h"
#include "MyMutex.h"



/*
struct inotify_event {
   int      wd;       // Watch descriptor
   uint32_t mask;     // Mask of events
   uint32_t cookie;   // Unique cookie associating related  events (for rename(2))
   uint32_t len;      // Size of name field
   char     name[];   // Optional null-terminated name
};

*/

static void* watchThreadD(void*  pData) {
	FileWatch* pWatch = (FileWatch*)pData;
	if (pWatch) {
		pWatch->DoWork();
	}
	return (void*)0;
}

int FileWatch::watch_inotify_events(int fd)
{
	char event_buf[512] = {0};
	int ret = 0;
	int event_pos = 0;
	int event_size = 0;
#ifndef WIN32
	struct inotify_event *event = NULL;

	/*���¼��Ƿ�����û�з����ͻ�����*/
	ret = read(fd, event_buf, sizeof(event_buf));

	/*���read�ķ���ֵ��С��inotify_event��С���ִ���*/
	if (ret < (int)sizeof(struct inotify_event))
	{
		printf("counld not get event!\n");
		return -1;
	}

	/*��Ϊread�ķ���ֵ����һ�����߶��inotify_event������Ҫһ��һ��ȡ��������*/
	while (ret >= (int)sizeof(struct inotify_event))
	{
		event = (struct inotify_event*)(event_buf + event_pos);
		if (event->len)
		{
			if (event->mask & IN_CREATE)
			{
				printf("create file: %s\n", event->name);
			}
			else if(event->mask & IN_DELETE)
			{
				printf("delete file: %s\n", event->name);
			}
		}

		/*event_size����һ���¼���������С*/
		event_size = sizeof(struct inotify_event) + event->len;
		ret -= event_size;
		event_pos += event_size;
	}
#endif

	return 0;
}

void FileWatch::readEvent(void* pEvent) {
	int nLen = 0;
	FileEvent fileEvent;

#ifndef WIN32
	struct inotify_event* pNotifyEvent = (struct inotify_event*)pEvent;
	memset(&fileEvent, 0x00, sizeof(fileEvent));

	if (pNotifyEvent->mask & IN_CREATE)
	{
		if (pNotifyEvent->mask & IN_ISDIR)
		{
			fileEvent.szEventType = EVENT_DIR_CREATE;
		}
		else {
			fileEvent.szEventType = EVENT_FILE_CREATE;
		}
		printf("create file: %s\n", pNotifyEvent->name);
	}
	else if (pNotifyEvent->mask & IN_DELETE)
	{
		if (pNotifyEvent->mask & IN_ISDIR)
		{
			fileEvent.szEventType = EVENT_DIR_DELETE;
		}
		else {
			fileEvent.szEventType = EVENT_FILE_DELETE;
		}
		printf("delete file: %s\n", pNotifyEvent->name);
	}

	nLen = strlen(pNotifyEvent->name);
	
	nLen = nLen > (MAX_PATH - 1) ? MAX_PATH - 1 : nLen;
	memcpy(fileEvent.szFileName, pNotifyEvent->name, nLen);
#endif
	if (m_pCb) {
		(*m_pCb)(&fileEvent, m_pObject);
	}

}

void FileWatch::DoWork() {

	if (m_nNotifyFd == -1) {
		startWatch(m_sPath.c_str());
	}
#ifndef WIN32	
	fd_set rfd = { 0 };
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 20000;//20millsecond

	while (!m_bExit)
	{
		int retval = 0;
		FD_ZERO(&rfd);
		FD_SET(m_nNotifyFd, &rfd);
		retval = select(m_nNotifyFd + 1, &rfd, NULL, NULL, &tv);
		if (retval == 0) 
			continue;
		else if (retval == -1) {
			printf("select Error %d  ", retval);
		}
		else {
			watch_inotify_events(m_nNotifyFd);
		}
	}
#endif 
}



void FileWatch::startWatch(const char* szPath) {
	int iNotifyFd = 0;
	int ret = 0;
#ifndef WIN32	
	/*inotify��ʼ��*/
	iNotifyFd = inotify_init();
	if (iNotifyFd == -1)
	{
		printf("inotify_init error!\n");
		return ;
	}

	/*���watch����*/
	ret = inotify_add_watch(iNotifyFd, szPath, IN_CREATE | IN_DELETE);
#endif
	/*�����¼�*/ /* ���������Բ����⴦��*/
	//watch_inotify_events(iNotifyFd);
	m_nNotifyFd = iNotifyFd;
}

int FileWatch::stopWatch(int iNotifyFd) {

	int ret = 0;
#ifndef WIN32	
	/*ɾ��inotify��watch����*/
	if (inotify_rm_watch(iNotifyFd, ret) == -1)
	{
		printf("notify_rm_watch error!\n");
		return -1;
	}

	/*�ر�inotify������*/
	close(iNotifyFd);
#endif
	return 0;
}

void FileWatch::Start() {
	m_bExit = 0;

	std::cout << "UploadTaskQueue Start " << std::endl;

	pthread_create(&m_watch_t, NULL, watchThreadD, (void*)this);

	startWatch(m_sPath.c_str());
}

void FileWatch::Stop() {
	m_bExit = 1;

	MSleep(20);

	std::cout << "Watch Thread Stop " << std::endl;
	pthread_join(m_watch_t, NULL);

	if (m_nNotifyFd != -1) {
		stopWatch(m_nNotifyFd);
		m_nNotifyFd = -1;
	}
	
}






