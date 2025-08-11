#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#include "baseServer/socketBase.h"
#include <unistd.h>
#include "baseServer/myAddr.h"

class TcpServer :
    public SocketBase
{
public:
    TcpServer(uint16_t portNumber, const char* IP, int type = SOCK_STREAM, int domain = AF_INET, int protocol = 0);
    ~TcpServer();
    void working();
    void stopConnect();
private:
    MyAddr *addr;
};

#endif