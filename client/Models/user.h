#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    User();
    User(int idUser, int idClient, const QString& role);

    int idUser() const;
    int idClient() const;
    QString role() const;
    bool isAdmin() const;
    bool isClient() const;

private:
    int m_idUser;
    int m_idClient;
    QString m_role;
};

#endif // USER_H
