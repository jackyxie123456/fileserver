#pragma once
#ifndef MSG_COMMON_H_
#define MSG_COMMON_H_
#include <string.h>  // strcpy
#include <stdio.h>
#include <sys/types.h>

#ifndef WIN32
#include <unistd.h>  // read
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#define PATHNAME "./"
#define PROJ_ID     0x666
#define MSGSIZE     1024

#define SERVER_TYPE 1   // ����˷�����Ϣ����
#define CLIENT_TYPE 2   // �ͻ��˷�����Ϣ����

struct msgbuf          // ��Ϣ�ṹ
{
	int mtype;     // ��Ϣ����
	int msize;     // ��Ϣ�峤��
	char mtext[MSGSIZE]; // ��Ϣbuf
};

int createMsgQueue();  // ������Ϣ����
int destroyMsgQueue(int msqid); // ������Ϣ����

int getMsgQueue();     //��ȡ��Ϣ����

int sendMsg(int msqid, long type, const char *_sendInfo);   // ������Ϣ
int recvMsg(int msqid, long type, char* buf);       // ������Ϣ

#endif /* _COMMON_H*/
