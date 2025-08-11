#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QString>
#include "chatwidget.h"
#include <QObject>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();
    void loginCheck();
signals:
    void successLogin(const QString &usr);

private slots:
    void on_loginBtn_released();
    void loginSuccess();
    void loginFail();
    void loginNotExit();
private:
    Ui::LoginWidget *ui;
    ChatWidget *m_chatWin;
    void *arg;
};

#endif // LOGINWIDGET_H
