#include "tcpServer.h"

TcpServer::TcpServer(uint16_t portNumber, const char* IP, int type, int domain, int protocol)
	:SocketBase(type, domain, protocol)
{
	this->addr = new MyAddr(portNumber, IP);
}

TcpServer::~TcpServer()
{
}

void TcpServer::working()
{
	if (bind(socketFd, this->addr->getAddr(), this->addr->getAddrLen()) < 0) {
		perror("bind is error\n");
        exit(EXIT_FAILURE);
	}
	if (listen(this->socketFd, 10) == -1) {
		perror("listen is error\n");
        exit(EXIT_FAILURE);
	}
}

void TcpServer::stopConnect()
{
	if (this->socketFd != 0 && this->socketFd > 0) {
		close(this->socketFd);
		this->getSocketFd(0);
	}
}
