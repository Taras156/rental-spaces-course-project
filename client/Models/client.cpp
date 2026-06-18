#include "client.h"

Client::Client() : m_idClient(0), m_idUser(0) {}
Client::Client(int idClient, int idUser, const QString& name, const QString& address,
               const QString& phone, const QString& requisites, const QString& contactPerson)
    : m_idClient(idClient), m_idUser(idUser), m_name(name), m_address(address),
      m_phone(phone), m_requisites(requisites), m_contactPerson(contactPerson) {}
int Client::idClient() const { return m_idClient; }
int Client::idUser() const { return m_idUser; }
QString Client::name() const { return m_name; }
QString Client::address() const { return m_address; }
QString Client::phone() const { return m_phone; }
QString Client::requisites() const { return m_requisites; }
QString Client::contactPerson() const { return m_contactPerson; }
