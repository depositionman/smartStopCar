#include "readthread.h"
#include <unistd.h>
#include <QDebug>

ReadThread::ReadThread()
{
    m_socketfd = 0;
    //初始化各个包体
    memset(&m_head,0,sizeof(m_head));
    memset(&m_LoginBackMsg,0,sizeof(m_LoginBackMsg));
    memset(&m_chatBackMsg,0,sizeof(m_chatBackMsg));
}

int ReadThread::getSocketfd() const
{
    return m_socketfd;
}

void ReadThread::setSocketfd(int socketfd)
{
    m_socketfd = socketfd;
}

//懒汉式单例
std::mutex ReadThread::m_mutex;
ReadThread* ReadThread::m_readThread = nullptr;
ReadThread *ReadThread::getInstance()
{
    //两次if判断防止每次都需要上锁，减少上锁时的开销
    if(ReadThread::m_readThread == nullptr){
        //加锁
        std::unique_lock<std::mutex> lock(m_mutex);
        if(ReadThread::m_readThread == nullptr){
            ReadThread::m_readThread = new ReadThread();
        }
    }

    return ReadThread::m_readThread;
}

void ReadThread::run()
{
    int res = 0;
    char buf[300] = { 0 };
    while (1) {
        res = read(m_socketfd, buf, sizeof(HEAD));
        if (res < 0) {
            perror("read is error\n");
            break;
        }
        else if (res == 0) {
            break;
        }
        memcpy(&m_head, buf, res);  //获取包头

        if (m_head.businessType == LOGINBACK) {//登录返回包
            memset(buf, 0, sizeof(buf));
            res = read(m_socketfd, buf, sizeof(BACKMSG));
            if (res < 0) {
                perror("read is error\n");
                break;
            }
            else if (res == 0) {
                break;
            }
            memcpy(&m_LoginBackMsg, buf, res);
            //进行登录的身份验证
            if(m_LoginBackMsg.flag == SUCCESS){
                qDebug() << "login success" << endl;
                emit send2LoginWinSuccess();
            }
            else if(m_LoginBackMsg.flag == FAIL){
                qDebug() << "login fail" << endl;
                emit send2LoginWinFail();
            }
            else{
                qDebug() << "usr is not exit" << endl;
                emit send2LoginWinNotExit();
            }
        }
        else if (m_head.businessType == CHATBACK) {//聊天返回包
//            qDebug()<<"get chat back"<<m_socketfd;
            memset(buf, 0, sizeof(buf));
            res = read(m_socketfd, buf, sizeof(CHATMSG));
//            qDebug()<<"get chat back"<<m_socketfd;
            if (res < 0) {
                perror("read is error\n");
                break;
            }
            else if (res == 0) {
                break;
            }
            memcpy(&m_chatBackMsg, buf, res);
            emit send2ChatWin(m_chatBackMsg.sendID,m_chatBackMsg.context);
            qDebug() << "send id:" << m_chatBackMsg.sendID << endl;
            qDebug() << "context:" << m_chatBackMsg.context << endl;
        }
    }
}
