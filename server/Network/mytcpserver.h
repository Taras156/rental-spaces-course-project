#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QList>
#include <QString>

class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit MyTcpServer(QObject* parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();

private:
    QTcpServer* mTcpServer;
    QList<QTcpSocket*> sockets;
    QHash<QTcpSocket*, QString> socketRoles;
    QHash<QTcpSocket*, int> socketUserIds;
    QHash<QTcpSocket*, int> socketClientIds;
    QHash<QTcpSocket*, QString> socketBuffers;
};

#endif // MYTCPSERVER_H
