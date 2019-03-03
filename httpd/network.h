#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifndef WIN32
#define SOCKET  int
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else 
#include <WinSock2.h>
#endif


#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif 

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

ret_code_t network_init();
ret_code_t network_unint();
ret_code_t network_listen(uint16_t *port, SOCKET *fd);
ret_code_t network_accept(SOCKET sfd, struct in_addr* addr, SOCKET *cfd);
ret_code_t network_read(SOCKET fd, char *buf, int32_t size);
ret_code_t network_write(SOCKET fd, void *buf, uint32_t size);

void close_socket(int socketId);//

#endif