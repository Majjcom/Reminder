#include "src/mainwindow.h"
#include "src/singleappservice.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    if (!SingleAppService::check_single())
    {
        return 0;
    }

    QApplication a(argc, argv);
    MainWindow* w = new MainWindow;
    w->show();
    int ret = a.exec();
    delete w;
    return ret;
}
