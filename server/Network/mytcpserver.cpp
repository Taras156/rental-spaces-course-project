#include "mytcpserver.h"
#include "../Database/databasemanager.h"
#include "../Handlers/requesthandler.h"

#include <QDebug>
#include <QHostAddress>
#include <QStringList>

MyTcpServer::MyTcpServer(QObject* parent)
    : QObject(parent),
      mTcpServer(new QTcpServer(this))
{
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33334))
        qDebug() << "Server is NOT started:" << mTcpServer->errorString();
    else
        qDebug() << "Server is started on port 33334";

    if (!DatabaseManager::connect())
        qDebug() << "DATABASE CONNECTION FAILED:" << DatabaseManager::lastError();
}

MyTcpServer::~MyTcpServer()
{
    for (QTcpSocket* socket : sockets)
    {
        socket->close();
        socket->deleteLater();
    }
    sockets.clear();

    if (mTcpServer)
        mTcpServer->close();

    DatabaseManager::disconnect();
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket* clientSocket = mTcpServer->nextPendingConnection();
    sockets.append(clientSocket);
    socketRoles.insert(clientSocket, QString());
    socketUserIds.insert(clientSocket, 0);
    socketClientIds.insert(clientSocket, 0);
    socketBuffers.insert(clientSocket, QString());

    connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);

    qDebug() << "New client connected";
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    socketBuffers[socket].append(QString::fromUtf8(socket->readAll()));

    while (socketBuffers[socket].contains(QChar(1)))
    {
        int separatorIndex = socketBuffers[socket].indexOf(QChar(1));
        QString request = socketBuffers[socket].left(separatorIndex).trimmed();
        socketBuffers[socket].remove(0, separatorIndex + 1);

        if (request.isEmpty())
            continue;

        QString& role = socketRoles[socket];
        int& userId = socketUserIds[socket];
        int& clientId = socketClientIds[socket];

        QString response = RequestHandler::processRequest(request, role, userId, clientId);
        qDebug() << "Request:" << request;
        qDebug() << "Response:" << response;

        socket->write(response.toUtf8());
        socket->write("\x01");
        socket->flush();
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    qDebug() << "Client disconnected";

    sockets.removeAll(socket);
    socketRoles.remove(socket);
    socketUserIds.remove(socket);
    socketClientIds.remove(socket);
    socketBuffers.remove(socket);

    socket->close();
    socket->deleteLater();
}
