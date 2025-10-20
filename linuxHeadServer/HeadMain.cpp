#include <iostream>
#include "tcpServer/epollServer.h"
using namespace std;

#define SERVER_PORT 10002
#define SERVER_IP   "192.168.91.128"

int main()
{
    system("clear");
    system("../cleanup_resources.sh");  // 启动前强制清理
    std::cout << "------------前置服务器启动-----------"<< std::endl;
    // 创建epoll去监听客户端并处理对应的业务
    EpollServer epollserver(SERVER_PORT,SERVER_IP);
    epollserver.working();
    
    return 0;
}