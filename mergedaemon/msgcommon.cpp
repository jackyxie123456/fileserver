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
	// ����IPC �ؼ���
	int msqid = -1;//

#ifndef WIN32
	key_t _k = ftok(PATHNAME, PROJ_ID);
	msqid = msgget(_k, msgflag); // ��ȡ��Ϣ����ID
	if (msqid < 0)
	{
		perror("msgget");
		return -2;
	}
#endif
	return msqid;

}


int createMsgQueue()  // ������Ϣ����
{
#ifndef WIN32
	return commMsg(IPC_CREAT | IPC_EXCL | 0666);
#endif
	return 0;
}

int destroyMsgQueue(int msqid) // ������Ϣ����
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

int getMsgQueue()     // ��ȡ��Ϣ����
{
#ifndef WIN32
	return commMsg(IPC_CREAT);
#endif
	return 0;
}

int sendMsg(int msqid, long type, const char *_sendInfo)         // ������Ϣ
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

int recvMsg(int msqid, long type, char* buf)          // ������Ϣ
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



