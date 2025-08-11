#ifndef SOCKETCONNECT_H
#define SOCKETCONNECT_H

#include <mutex> // std::mutex
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr的头文件
#include <unistd.h>

class SocketConnect
{
public:
    //提供接口获取对象
    static SocketConnect* getInstance();
    void socketConnect();
    void socketDeploy(sa_family_t socketClan, uint16_t portNumber, const char *IP);
    void closeSocketConnect();
    int getSocketfd() const;

private:
    //构造函数私有化
    SocketConnect();
    //实例对象
    static SocketConnect *m_SocketConnect;
    static std::mutex m_mutex;
    int m_socketfd;
    socklen_t m_clilen;
    struct sockaddr_in m_addr;
};

#endif // SOCKETCONNECT_H
