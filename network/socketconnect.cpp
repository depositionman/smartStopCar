#include "socketconnect.h"
#include <QDebug>

//懒汉式单例
std::mutex SocketConnect::m_mutex;
SocketConnect *SocketConnect::m_SocketConnect = nullptr;
SocketConnect *SocketConnect::getInstance()
{
    //两次if判断防止每次都需要上锁，减少上锁时的开销
    if(SocketConnect::m_SocketConnect == nullptr){
        //加锁
        std::unique_lock<std::mutex> lock(m_mutex);
        if(SocketConnect::m_SocketConnect == nullptr){
            SocketConnect::m_SocketConnect = new SocketConnect();
        }
    }

    return SocketConnect::m_SocketConnect;
}

void SocketConnect::socketConnect()
{
    //选择TCP为通信协议
    m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketfd < 0) {
        perror("socket is error\n");
        return;
    }
    m_clilen = sizeof(m_addr);
    //连接服务器
    if (connect(m_socketfd, reinterpret_cast<struct sockaddr*>(&m_addr), sizeof(m_addr)) < 0) {
        perror("connect is error\n");
        close(m_socketfd); // 关闭套接字
        return;
    }
}

void SocketConnect::socketDeploy(sa_family_t socketClan, uint16_t portNumber, const char *IP)
{
    //绑定网络端口与网络IP地址
    m_addr.sin_family = socketClan;
    m_addr.sin_port = htons(portNumber);
    m_addr.sin_addr.s_addr = inet_addr(IP);
}

void SocketConnect::closeSocketConnect()
{

}

SocketConnect::SocketConnect()
{

}

int SocketConnect::getSocketfd() const
{
    return m_socketfd;
}
