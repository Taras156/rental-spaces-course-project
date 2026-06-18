#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Client
{
public:
    Client();
    Client(int idClient, int idUser, const QString& name, const QString& address,
           const QString& phone, const QString& requisites, const QString& contactPerson);

    int idClient() const;
    int idUser() const;
    QString name() const;
    QString address() const;
    QString phone() const;
    QString requisites() const;
    QString contactPerson() const;

private:
    int m_idClient;
    int m_idUser;
    QString m_name;
    QString m_address;
    QString m_phone;
    QString m_requisites;
    QString m_contactPerson;
};

#endif // CLIENT_H
