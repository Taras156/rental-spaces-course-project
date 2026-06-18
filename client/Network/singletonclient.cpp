#include "singletonclient.h"

#include <QDebug>

SingletonClient* SingletonClient::p_instance = nullptr;
SingletonClientDestroyer SingletonClient::destroyer;

SingletonClientDestroyer::~SingletonClientDestroyer()
{
    delete p_instance;
}

void SingletonClientDestroyer::initialize(SingletonClient* p)
{
    p_instance = p;
}

SingletonClient::SingletonClient(QObject* parent)
    : QObject(parent),
      m_pSocket(new QTcpSocket(this))
{
    connect(m_pSocket, &QTcpSocket::readyRead, this, &SingletonClient::slotReadyRead);
    connect(m_pSocket, &QTcpSocket::connected, this, &SingletonClient::slotConnected);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &SingletonClient::slotDisconnected);
    connect(m_pSocket, &QTcpSocket::errorOccurred, this, &SingletonClient::slotErrorOccurred);
}

SingletonClient::~SingletonClient()
{
    if (m_pSocket && m_pSocket->isOpen())
        m_pSocket->close();
}

SingletonClient* SingletonClient::getInstance()
{
    if (!p_instance)
    {
        p_instance = new SingletonClient();
        destroyer.initialize(p_instance);
    }
    return p_instance;
}

void SingletonClient::connectToServer(const QString& host, quint16 port)
{
    if (m_pSocket->state() == QAbstractSocket::ConnectedState)
        return;
    m_pSocket->connectToHost(host, port);
}

void SingletonClient::disconnectFromServer()
{
    if (m_pSocket->isOpen())
        m_pSocket->disconnectFromHost();
}

void SingletonClient::sendMessageToServer(const QString& query)
{
    if (m_pSocket->state() != QAbstractSocket::ConnectedState)
    {
        emit errorOccurred("Нет подключения к серверу. Сначала запустите rental_spaces_server.");
        return;
    }

    m_pSocket->write(query.toUtf8());
    m_pSocket->write("\x01");
    m_pSocket->flush();
}

bool SingletonClient::isConnected() const
{
    return m_pSocket->state() == QAbstractSocket::ConnectedState;
}

void SingletonClient::slotReadyRead()
{
    m_buffer.append(QString::fromUtf8(m_pSocket->readAll()));

    while (m_buffer.contains(QChar(1)))
    {
        int separatorIndex = m_buffer.indexOf(QChar(1));
        QString message = m_buffer.left(separatorIndex).trimmed();
        m_buffer.remove(0, separatorIndex + 1);

        if (!message.isEmpty())
            emit messageFromServer(message);
    }
}

void SingletonClient::slotConnected()
{
    emit connected();
}

void SingletonClient::slotDisconnected()
{
    emit disconnected();
}

void SingletonClient::slotErrorOccurred(QAbstractSocket::SocketError)
{
    emit errorOccurred(m_pSocket->errorString());
}
