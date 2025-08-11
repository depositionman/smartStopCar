#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>
#include "agreement/protocol.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr的头文件
#include "network/socketconnect.h"
#include "thread/readthread.h"
#include "thread/writethread.h"
#include <QDebug>

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    m_chatWin = nullptr;
}

LoginWidget::~LoginWidget()
{
    delete ui;
    if(m_chatWin != nullptr){
        delete m_chatWin;
        m_chatWin = nullptr;
    }
}

void LoginWidget::loginCheck()
{
    char buf[300] = {0};
    //将登录的信息发送给服务器
    HEAD head;
    memset(&head,0,sizeof(head));
    USER user;
    memset(&user,0,sizeof(user));
    //写包头
    head.businessType = LOGINREQUEST;
    head.businessLength = sizeof(USER);
    //写包体
    user.userID = atoi(ui->usrEdit->text().toStdString().c_str());
    strcpy(user.password,ui->pwdEdit->text().toStdString().c_str());
    //发送登录请求包
    memcpy(buf,&head,sizeof(head));
    memcpy(buf+sizeof(head),&user,sizeof(user));
    WriteThread::getInstance()->setData(buf,sizeof(head)+sizeof(user));
//    qDebug()<<head.businessType<<head.businessLength;
//    qDebug()<<user.userID<<user.password;
}

void LoginWidget::on_loginBtn_released()
{
    //判断用户是否存在并记录用户的数据
    //表单不能为空
    if(ui->usrEdit->text() == ""){
        QMessageBox::warning(this,"warning","usrEdit is empty,login error");
    }
    else if(ui->pwdEdit->text() == ""){
        QMessageBox::warning(this,"warning","pwdEdit is empty,login error");
    }
    //进行网络的配置和连接
    SocketConnect::getInstance()->socketDeploy(AF_INET,10010,"192.168.126.128");
    SocketConnect::getInstance()->socketConnect();
    //开启读写线程
    int socketfd = SocketConnect::getInstance()->getSocketfd();
    ReadThread::getInstance()->setSocketfd(socketfd);
    WriteThread::getInstance()->setSocketfd(socketfd);

    ReadThread::getInstance()->start();
    connect(ReadThread::getInstance(),SIGNAL(send2LoginWinSuccess()),this,SLOT(loginSuccess()));
    connect(ReadThread::getInstance(),SIGNAL(send2LoginWinFail()),this,SLOT(loginFail()));
    connect(ReadThread::getInstance(),SIGNAL(send2LoginWinNotExit()),this,SLOT(loginNotExit()));
    WriteThread::getInstance()->start();

    loginCheck();
}

void LoginWidget::loginSuccess()
{
    QMessageBox::information(this,"info","login is success");
    //创建聊天窗口
    this->close();
    m_chatWin = new ChatWidget();
    m_chatWin->setLoginUsrID(atoi(ui->usrEdit->text().toStdString().c_str()));
    m_chatWin->show();
}

void LoginWidget::loginFail()
{
    QMessageBox::information(this,"warning","password is error");
}

void LoginWidget::loginNotExit()
{
    QMessageBox::information(this,"warning","usr is not exit");
}
