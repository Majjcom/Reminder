#ifndef SINGLEAPPSERVICE_H
#define SINGLEAPPSERVICE_H

#include <QObject>
#include <QTcpServer>

class MainWindow;

class SingleAppService : public QObject
{
    Q_OBJECT
public:
    explicit SingleAppService(MainWindow *parent = nullptr);
    static bool check_single();

private:
    MainWindow* mw;
    QTcpServer* server;

private slots:
    void new_conn();

signals:

};

#endif // SINGLEAPPSERVICE_H
