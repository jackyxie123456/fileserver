#include <iostream>

#ifndef WIN32
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#else 
#include <WinSock2.h>

#endif

#include <stdio.h>
#include <pthread.h>

#include <errno.h>

#define MAXLINE 10
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 8006
#define INFTIM 1000

//�̳߳�������нṹ��

struct task {
	int fd; //��Ҫ��д���ļ�������

	struct task *next; //��һ������

};

//���ڶ�д�������������洫�ݲ���
struct user_data {
	int fd;
	unsigned int n_size;
	char line[MAXLINE];
};

//�̵߳�������

void * readtask(void *args);
void * writetask(void *args);


//����epoll_event�ṹ��ı���,ev����ע���¼�,�������ڻش�Ҫ������¼�

struct epoll_event ev, events[20];
int epfd;
pthread_mutex_t mutex;
pthread_cond_t cond1;
struct task *readhead = NULL, *readtail = NULL, *writehead = NULL;

void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}
}

int main()
{
	int i, maxi, listenfd, connfd, sockfd, nfds;
	pthread_t tid1, tid2;

	struct task *new_task = NULL;
	struct user_data *rdata = NULL;

	socklen_t clilen;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond1, NULL);
	//��ʼ�����ڶ��̳߳ص��߳�

	pthread_create(&tid1, NULL, readtask, NULL);
	pthread_create(&tid2, NULL, readtask, NULL);

	//�������ڴ���accept��epollר�õ��ļ������� 

	epfd = epoll_create(256);

	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//��socket����Ϊ��������ʽ

	setnonblocking(listenfd);
	//������Ҫ������¼���ص��ļ�������

	ev.data.fd = listenfd;
	//����Ҫ������¼�����

	ev.events = EPOLLIN | EPOLLET;
	//ע��epoll�¼�

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, LISTENQ);

	maxi = 0;
	for (; ; ) {
		//�ȴ�epoll�¼��ķ���

		nfds = epoll_wait(epfd, events, 20, 500);
		//�����������������¼� 

		for (i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == listenfd)
			{

				connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
				if (connfd < 0) {
					perror("connfd < 0");
					exit(1);
				}
				setnonblocking(connfd);

				char *str = inet_ntoa(clientaddr.sin_addr);
				//std::cout<<"connec_ from >>"<<str<<std::endl;

				//�������ڶ��������ļ�������

				ev.data.fd = connfd;
				//��������ע��Ķ������¼�

				ev.events = EPOLLIN | EPOLLET;
				//ע��ev

				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			}
			else if (events[i].events&EPOLLIN)
			{
				//printf("reading!/n"); 

				if ((sockfd = events[i].data.fd) < 0) 
					continue;

				new_task = new task();
				new_task->fd = sockfd;
				new_task->next = NULL;
				//����µĶ�����

				pthread_mutex_lock(&mutex);
				if (readhead == NULL)
				{
					readhead = new_task;
					readtail = new_task;
				}
				else
				{
					readtail->next = new_task;
					readtail = new_task;
				}
				//�������еȴ�cond1�������߳�

				pthread_cond_broadcast(&cond1);
				pthread_mutex_unlock(&mutex);
			}
			else if (events[i].events&EPOLLOUT)
			{
				/*
			 rdata = (struct user_data *)events[i].data.ptr;
				sockfd = rdata->fd;
				write(sockfd, rdata->line, rdata->n_size);
				delete rdata;
				//�������ڶ��������ļ�������
				ev.data.fd = sockfd;
				//��������ע��Ķ������¼�
			  ev.events = EPOLLIN|EPOLLET;
				//�޸�sockfd��Ҫ������¼�ΪEPOLIN
			  epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
			*/
			}

		}

	}
}

static int count111 = 0;
static time_t oldtime = 0, nowtime = 0;
void * readtask(void *args)
{

	int fd = -1;
	unsigned int n;
	//���ڰѶ����������ݴ��ݳ�ȥ
	struct user_data *data = NULL;
	while (1) {

		pthread_mutex_lock(&mutex);
		//�ȴ���������в�Ϊ��

		while (readhead == NULL)
			pthread_cond_wait(&cond1, &mutex);

		fd = readhead->fd;
		//���������ȡ��һ��������

		struct task *tmp = readhead;
		readhead = readhead->next;
		delete tmp;
		pthread_mutex_unlock(&mutex);
		data = new user_data();
		data->fd = fd;

		char recvBuf[1024] = { 0 };
		int ret = 999;
		int rs = 1;

		while (rs)
		{
			ret = recv(fd, recvBuf, 1024, 0);// ���ܿͻ�����Ϣ

			if (ret < 0)
			{
				//�����Ƿ�������ģʽ,���Ե�errnoΪEAGAINʱ,��ʾ��ǰ�������������ݿ�//��������͵����Ǹô��¼��Ѵ������
				if (errno == EAGAIN)
				{
					printf("EAGAIN\n");
					break;
				}
				else {
					printf("recv error!\n");

					close(fd);
					break;
				}
			}
			else if (ret == 0)
			{
				// �����ʾ�Զ˵�socket�������ر�. 
				rs = 0;
			}
			if (ret == sizeof(recvBuf))
				rs = 1; // ��Ҫ�ٴζ�ȡ
			else
				rs = 0;
		}
		if (ret > 0) {

			//-------------------------------------------------------------------------------
			data->n_size = n;


			count111++;

			struct tm *today;
			time_t ltime;
			time(&nowtime);

			if (nowtime != oldtime) {
				printf("%d\n", count111);
				oldtime = nowtime;
				count111 = 0;
			}

			char buf[1000] = { 0 };
			sprintf(buf, "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n%s", "Hello world!\n");
			send(fd, buf, strlen(buf), 0);
			close(fd);


		}
	}
}