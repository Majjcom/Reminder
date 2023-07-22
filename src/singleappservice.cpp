#include "singleappservice.h"

#include "mainwindow.h"

#include <QHostAddress>
#include <QTcpSocket>

SingleAppService::SingleAppService(MainWindow *parent)
    : QObject{parent}
    , mw(parent)
{
    server = new QTcpServer(this);
    server->listen(QHostAddress::LocalHost, 15660);

    connect(
        server, SIGNAL(newConnection()),
        this, SLOT(new_conn())
    );
}

bool SingleAppService::check_single()
{
    QTcpSocket* s = new QTcpSocket;
    s->connectToHost("127.0.0.1", 15660);
    bool ok = s->waitForConnected(500);
    if (!ok)
    {
        s->disconnectFromHost();
        s->deleteLater();
        return true;
    }
    s->write("ReminderShowUp");
    ok = s->waitForReadyRead(500);
    if (!ok)
    {
        s->disconnectFromHost();
        s->deleteLater();
        return true;
    }
    if (QString::fromUtf8(s->read(2)).startsWith("ok"))
    {
        s->disconnectFromHost();
        s->deleteLater();
        return false;
    }
    s->disconnectFromHost();
    s->deleteLater();
    return true;
}

void SingleAppService::new_conn()
{
    QTcpSocket* s = server->nextPendingConnection();
    bool ok = s->waitForReadyRead(500);
    if (!ok)
    {
        s->disconnectFromHost();
        s->deleteLater();
        return;
    }
    if (QString::fromUtf8(s->readAll()).startsWith("ReminderShowUp"))
    {
        s->write("ok");
        s->disconnectFromHost();
        s->deleteLater();
        mw->show_window();
        return;
    }
    s->disconnectFromHost();
    s->deleteLater();
    return;
}
