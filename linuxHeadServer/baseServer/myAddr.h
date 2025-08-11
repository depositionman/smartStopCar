#ifndef __MYADDR_H
#define __MYADDR_H

#include <sys/types.h>        
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

class MyAddr
{
public:
    MyAddr(uint16_t portNumber, const char *IP, sa_family_t socketClan = AF_INET);
    ~MyAddr();

    socklen_t getAddrLen();
	uint16_t getPortNumber();
	void setPortNumber(uint16_t portNumber);

	struct sockaddr* getAddr();
	struct sockaddr_in* getAddr_in();
protected:
private:
    struct sockaddr_in addr;
    socklen_t addrLen;
    uint16_t portNumber;
};

#endif