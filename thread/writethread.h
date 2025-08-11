#ifndef WRITETHREAD_H
#define WRITETHREAD_H

#include <QObject>
#include <mutex>
#include <QThread>
#include "agreement/protocol.h"

class WriteThread : public QThread
{
    Q_OBJECT
public:
    static WriteThread* m_writeThread;
    static WriteThread* getInstance();
    void run();
    void setData(void *data, int len);
    int getRcvID() const;
    void setRcvID(int rcvID);

    int getSocketfd() const;
    void setSocketfd(int socketfd);

signals:

public slots:
private:
    WriteThread();
    static std::mutex m_mutex;
    HEAD m_head;
    USER m_usr;
    CHATMSG m_chatMsg;
    int m_socketfd;
    int m_len;  //发送数据的长度
    int m_rcvID;
    char m_context[150];
    char m_buf[300];
};

#endif // WRITETHREAD_H
