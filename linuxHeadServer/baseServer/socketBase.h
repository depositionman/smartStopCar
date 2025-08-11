#ifndef __SOCKETBASE_H
#define __SOCKETBASE_H

#include <sys/types.h>    
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>

class SocketBase
{
public:
	SocketBase(int type, int domain = AF_INET, int protocol = 0);
	virtual ~SocketBase();
	int getSocketFd();
	int getSocketType();
	void getSocketFd(int socketFd);
	virtual void stopConnect() = 0;
	virtual void working() = 0;
protected:
	int socketFd;
	int type;
};

#endif