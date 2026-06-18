#include "user.h"

User::User()
    : m_idUser(0), m_idClient(0)
{
}

User::User(int idUser, int idClient, const QString& role)
    : m_idUser(idUser), m_idClient(idClient), m_role(role)
{
}

int User::idUser() const { return m_idUser; }
int User::idClient() const { return m_idClient; }
QString User::role() const { return m_role; }
bool User::isAdmin() const { return m_role == "Администратор"; }
bool User::isClient() const { return m_role == "Клиент"; }
