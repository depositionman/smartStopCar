#include "epollServer.h"
#include <iostream>
#include "client/clientTask.h"
#include <fcntl.h>  // 新增头文件

using namespace std;

EpollServer::EpollServer(uint16_t portNumber, const char* IP)
	: tcpServer(new TcpServer(portNumber, IP)), threadPool(3)
{
	this->tcpServer->working();
	//从tcpServer中获取socket套接字
	this->socketfd = this->tcpServer->getSocketFd();
	//初始化所有成员参数
	this->epollWaitNum = 0;
	this->epollfd = epoll_create(EPOLL_SIZE);
	this->acceptfd = 0;

	memset(this->buf, 0, sizeof(this->buf));
	memset(&this->epollEvent, 0, sizeof(this->epollEvent));
	memset(this->epollEventArray, 0, sizeof(this->epollEventArray));
	//将tcp套接字放入epoll中
	control(EPOLL_CTL_ADD, this->socketfd, LT_MODE);
}

EpollServer::~EpollServer()
{
	delete this->tcpServer;
}

//LT模式：当套接字有数据时如果你没有取走数据，系统会一直通知你
//ET模式：当套接字有数据系统只会通知你一次，后面就不会通知了，直到有新数据的到来
void EpollServer::control(int opt, int fd, int type)
{
	this->epollEvent.data.fd = fd;
	if (type == LT_MODE) {//LT模式
		this->epollEvent.events = EPOLLIN;
	}
	else if (type == ET_MODE) { 
		this->epollEvent.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(this->epollfd, opt, fd, &this->epollEvent);
}

void EpollServer::working()
{
	while (1) {
		this->epollWaitNum = epoll_wait(this->epollfd, this->epollEventArray, EPOLL_SIZE, -1);
		if (this->epollWaitNum < 0) {
			perror("epoll_wait is error\n");
			return;
		}
		for (int i = 0; i < epollWaitNum; ++i) {
			if (this->epollEventArray[i].data.fd == socketfd) {//服务器的操作
				this->acceptfd = accept(socketfd, NULL, NULL);
				if (this->acceptfd == -1) {
					perror("accept is error\n");
					return;
				}
				//接收到客户端后 将各个客户端的fd加入到epoll里面
				this->control(EPOLL_CTL_ADD, this->acceptfd, ET_MODE);
				cout << "客户端连接成功，文件描述符为：" << this->acceptfd << endl;
			}
			else if (this->epollEventArray[i].events & EPOLLIN) {//客户端的操作
				memset(buf, 0, sizeof(buf));
				if (read(this->epollEventArray[i].data.fd, buf, sizeof(buf)) < 0) {//客户端异常退出
					this->control(EPOLL_CTL_DEL, this->epollEventArray[i].data.fd, ET_MODE);
					cout << "有客户端退出文件描述符为：" << this->epollEventArray[i].data.fd << endl;
				}
				else {
					ClientTask* clientTask = new ClientTask(this->epollEventArray[i].data.fd, buf);
					this->threadPool.pushTask(clientTask);
				}
			}
		}
	}
}