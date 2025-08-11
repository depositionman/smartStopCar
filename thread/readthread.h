#ifndef READTHREAD_H
#define READTHREAD_H

#include <QObject>
#include <QThread>
#include <mutex>
#include "agreement/protocol.h"

class ReadThread : public QThread
{
    Q_OBJECT
public:
    static ReadThread* m_readThread;
    static ReadThread* getInstance();
    void run();
    int getSocketfd() const;
    void setSocketfd(int socketfd);

signals:
    void send2LoginWinSuccess();
    void send2LoginWinFail();
    void send2LoginWinNotExit();
    void send2ChatWin(int,char*);
public slots:
private:
    ReadThread();
    static std::mutex m_mutex;
    HEAD m_head;
    BACKMSG m_LoginBackMsg;
    CHATMSG m_chatBackMsg;
    int m_socketfd;
};

#endif // READTHREAD_H
