#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QString>
#include <QObject>

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();
    int loginUsrID() const;
    void setLoginUsrID(int loginUsrID);
    void sendChatMsg();

private slots:
    void on_pushButton_released();
    void updateWin(int sendID,char *context);
private:
    Ui::ChatWidget *ui;
    int m_loginUsrID;
};

#endif // CHATWIDGET_H
