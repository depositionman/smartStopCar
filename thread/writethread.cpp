#include "writethread.h"
#include <unistd.h>
#include <QDebug>
#include <string.h>

WriteThread::WriteThread()
{
    m_len = 0;
    m_socketfd = 0;
    m_rcvID = 0;
    m_buf[300] = {0};
    m_context[150] = {0};
    memset(m_context,0,sizeof(m_context));
    //初始化各个包体
    memset(&m_head,0,sizeof(m_head));
    memset(&m_usr,0,sizeof(m_usr));
    memset(&m_chatMsg,0,sizeof(m_chatMsg));
}

int WriteThread::getSocketfd() const
{
    return m_socketfd;
}

void WriteThread::setSocketfd(int socketfd)
{
    m_socketfd = socketfd;
}

int WriteThread::getRcvID() const
{
    return m_rcvID;
}

void WriteThread::setRcvID(int rcvID)
{
    m_rcvID = rcvID;
}

//懒汉式单例
std::mutex WriteThread::m_mutex;
WriteThread* WriteThread::m_writeThread = nullptr;
WriteThread *WriteThread::getInstance()
{
    //两次if判断防止每次都需要上锁，减少上锁时的开销
    if(WriteThread::m_writeThread == nullptr){
        //加锁
        std::unique_lock<std::mutex> lock(m_mutex);
        if(WriteThread::m_writeThread == nullptr){
            WriteThread::m_writeThread = new WriteThread();
        }
    }

    return WriteThread::m_writeThread;
}

void WriteThread::run()
{
    //向服务器发送数据
    int res = 0;
    while (1) {
        if(m_len > 0){
//            qDebug() << m_len;
//            qDebug() <<m_head.businessType;
            if(m_head.businessType == LOGINREQUEST){
                if (write(m_socketfd, m_buf, sizeof(HEAD) + sizeof(USER)) < 0) {
                    perror("write is error\n");
                    close(m_socketfd); // 关闭套接字
                }
                memset(m_buf,0,sizeof(m_buf));
            }
            else if(m_head.businessType == CHATREQUEST){
                if (write(m_socketfd, m_buf, sizeof(HEAD) + sizeof(CHATMSG)) < 0) {
                    perror("write is error\n");
                    close(m_socketfd); // 关闭套接字
                }
                memset(m_buf,0,sizeof(m_buf));
            }
            m_len = 0;
        }
    }
}

void WriteThread::setData(void *data, int len)
{
    if(len > 0){
        memset(m_buf,0,sizeof(m_buf));
        memcpy(m_buf,data,len);
        memcpy(&m_head,m_buf,sizeof(m_head));
        m_len = len;
//        qDebug()<<m_len;
//        qDebug() <<m_head.businessType;
    }
}
