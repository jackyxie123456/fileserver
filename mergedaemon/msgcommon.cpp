#include "msgcommon.h"


#ifdef WIN32

int msgsnd(int msqid, const void* msgp, size_t msgsz, int msgflag) {
	return 0;
}
int msgrcv(int msqid, void *ptr, size_t nbytes, long type, int msgflag) {
	return 0;
}

int msgctl(int msqid, int cmd, void *buf) {
	return 0;
}

#endif

int commMsg(int msgflag)
{
	// 生成IPC 关键字
	int msqid = -1;//

#ifndef WIN32
	key_t _k = ftok(PATHNAME, PROJ_ID);
	msqid = msgget(_k, msgflag); // 获取消息队列ID
	if (msqid < 0)
	{
		perror("msgget");
		return -2;
	}
#endif
	return msqid;

}


int createMsgQueue()  // 创建消息队列
{
#ifndef WIN32
	return commMsg(IPC_CREAT | IPC_EXCL | 0666);
#endif
	return 0;
}

int destroyMsgQueue(int msqid) // 销毁消息队列
{
#ifndef WIN32
	int _ret = msgctl(msqid, IPC_RMID, 0);
	if (_ret < 0)
	{
		perror("msgctl");
		return -1;
	}
#endif 
	return 0;
}

int getMsgQueue()     // 获取消息队列
{
#ifndef WIN32
	return commMsg(IPC_CREAT);
#endif
	return 0;
}

int sendMsg(int msqid, long type, const char *_sendInfo)         // 发送消息
{
	struct msgbuf msg;
	msg.mtype = type;
	strcpy(msg.mtext, _sendInfo);
	
	int _snd = msgsnd(msqid, &msg, sizeof(msg), 0);
	if (_snd < 0)
	{
		perror("msgsnd");
		return -1;
	}
	return 0;
}

int recvMsg(int msqid, long type, char* buf)          // 接收消息
{
	struct msgbuf msg;
	int _rcv = msgrcv(msqid, &msg, sizeof(msg), type, 0);
	if (_rcv < 0)
	{
		perror("msgrcv");
		return -1;

	}
	strcpy(buf, msg.mtext);
	return 0;
}



