#include "chatwidget.h"
#include "ui_chatwidget.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr的头文件
#include <unistd.h>
#include <pthread.h>
#include "agreement/protocol.h"
#include <string.h>
#include "thread/readthread.h"
#include "thread/writethread.h"
#include "network/socketconnect.h"
#include <QDebug>

ChatWidget::ChatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWidget)
{
    ui->setupUi(this);
    m_loginUsrID = 0;
    //开启读写线程
    int socketfd = SocketConnect::getInstance()->getSocketfd();
    ReadThread::getInstance()->setSocketfd(socketfd);
    WriteThread::getInstance()->setSocketfd(socketfd);
//        qDebug()<<socketfd;

    ReadThread::getInstance()->start();
    bool rescon = connect(ReadThread::getInstance(),SIGNAL(send2ChatWin(int,char*)),this,SLOT(updateWin(int,char*)));
    qDebug()<<"rescon"<<rescon;
    WriteThread::getInstance()->start();
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

int ChatWidget::loginUsrID() const
{
    return m_loginUsrID;
}

void ChatWidget::setLoginUsrID(int loginUsrID)
{
    m_loginUsrID = loginUsrID;
}

void ChatWidget::sendChatMsg()
{
    char buf[300] = {0};
    //将登录的信息发送给服务器
    HEAD head;
    memset(&head,0,sizeof(head));
    CHATMSG chatMsg;
    memset(&chatMsg,0,sizeof(chatMsg));
    //写包头
    head.businessType = CHATREQUEST;
    head.businessLength = sizeof(chatMsg);
    //写包体
    chatMsg.sendID = m_loginUsrID;
//    qDebug()<<m_loginUsrID;
    int currentRow = ui->listWidget->currentRow();
    if (currentRow != -1) { // 检查是否有选中项
        QListWidgetItem* item = ui->listWidget->item(currentRow);
        chatMsg.rcvID = atoi(item->text().toStdString().c_str()); // 获取选中项的文本
    }
//    qDebug()<<chatMsg.rcvID;
    strcpy(chatMsg.context,ui->textEdit->toPlainText().toStdString().c_str());
//    qDebug()<<chatMsg.context;
    //发送登录请求包
    memcpy(buf,&head,sizeof(head));
    memcpy(buf+sizeof(head),&chatMsg,sizeof(chatMsg));
    WriteThread::getInstance()->setData(buf,sizeof(head)+sizeof(chatMsg));
}

void ChatWidget::updateWin(int sendID,char *context)
{
    QString text = "";
    if(sendID == m_loginUsrID){
       text = "I Say:";
       ui->textEdit->clear();
    }
    else {
       text = QString::number(sendID) + " say:";
    }
    text += context;
    ui->textEdit_2->append(text);
}

void ChatWidget::on_pushButton_released()
{
    //聊天编辑框有内容
    if(ui->textEdit->toPlainText() != ""){
//        qDebug()<<"ceshi";
        //发送信息给服务器
        sendChatMsg();
    }
}
