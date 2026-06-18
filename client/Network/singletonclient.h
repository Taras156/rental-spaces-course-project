#ifndef SINGLETONCLIENT_H
#define SINGLETONCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class SingletonClient;

class SingletonClientDestroyer
{
private:
    SingletonClient* p_instance = nullptr;
public:
    ~SingletonClientDestroyer();
    void initialize(SingletonClient* p);
};

class SingletonClient : public QObject
{
    Q_OBJECT

private:
    static SingletonClient* p_instance;
    static SingletonClientDestroyer destroyer;

    QTcpSocket* m_pSocket;
    QString m_buffer;

    explicit SingletonClient(QObject* parent = nullptr);
    SingletonClient(const SingletonClient&) = delete;
    SingletonClient& operator=(const SingletonClient&) = delete;
    ~SingletonClient();

    friend class SingletonClientDestroyer;

public:
    static SingletonClient* getInstance();

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    void sendMessageToServer(const QString& query);
    bool isConnected() const;

public slots:
    void slotReadyRead();
    void slotConnected();
    void slotDisconnected();
    void slotErrorOccurred(QAbstractSocket::SocketError error);

signals:
    void messageFromServer(const QString& msg);
    void connected();
    void disconnected();
    void errorOccurred(const QString& err);
};

#endif // SINGLETONCLIENT_H
