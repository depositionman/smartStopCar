#ifndef __EPOLLSERVER_H
#define __EPOLLSERVER_H
#include "tcpServer.h"
#include <sys/epoll.h>
#include "threadPool/threadPool.h"

#define EPOLL_SIZE 5
#define LT_MODE 1
#define ET_MODE 2

class EpollServer
{
public:
	EpollServer(uint16_t portNumber, const char* IP);
	~EpollServer();
	void control(int opt, int fd, int type);
	void working();
private:
	int socketfd;									
	int epollfd;
	int acceptfd;
	int epollWaitNum;
	struct epoll_event epollEvent;
	struct epoll_event epollEventArray[EPOLL_SIZE];
	char buf[6000];
	TcpServer* tcpServer;
	ThreadPool threadPool;
};

#endif  // __EPOLLSERVER_H

