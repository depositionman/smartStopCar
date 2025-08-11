#include "myAddr.h"

MyAddr::MyAddr(uint16_t portNumber, const char *IP, sa_family_t socketClan)
{
    this->portNumber = portNumber;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = socketClan;
    /* htons()作用是将端口号由主机字节序转换为网络字节序的整数值。(host to net)s是short的意思uint16_t
    htonl()作用和htons()一样，不过它针对的是32位的（long），而htons()针对的是两个字节，16位的（short）
    inet_addr()作用是将一个IP字符串转化为一个网络字节序的整数值，用于sockaddr_in.sin_addr.s_addr*/
    addr.sin_port = htons(this->portNumber);
    addr.sin_addr.s_addr = inet_addr(IP);

    addrLen = sizeof(addr);
}

MyAddr::~MyAddr()
{

}

socklen_t MyAddr::getAddrLen()
{
	return addrLen;
}


uint16_t MyAddr::getPortNumber()
{
	return this->portNumber;
}

void MyAddr::setPortNumber(uint16_t portNumber)
{
	this->portNumber = portNumber;
}

sockaddr* MyAddr::getAddr()
{
	return (sockaddr*)&this->addr;
}

sockaddr_in* MyAddr::getAddr_in()
{
	return &this->addr;
}