#include "socketBase.h"

SocketBase::SocketBase(int type, int domain, int protocol)
{
	this->socketFd = socket(domain, type, protocol);
	if (this->socketFd < 0) {
		perror("socket is error\n");
	}

	this->type = type;
}

SocketBase::~SocketBase()
{
}

int SocketBase::getSocketFd()
{
	return this->socketFd;
}

int SocketBase::getSocketType()
{
	return this->type;
}

void SocketBase::getSocketFd(int socketFd)
{
	this->socketFd = socketFd;
}
