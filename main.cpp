#include "view/loginwidget.h"
#include <QApplication>
#include "view/chatwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWidget w;
    w.show();
//    ChatWidget c;
//    c.show();

    return a.exec();
}
