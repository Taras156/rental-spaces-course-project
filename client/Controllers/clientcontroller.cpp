#include "clientcontroller.h"
#include "../Network/singletonclient.h"

#include <QUrl>

ClientController* ClientController::instance()
{
    static ClientController controller;
    return &controller;
}

ClientController::ClientController(QObject* parent)
    : QObject(parent)
{
}

QString ClientController::encodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

void ClientController::loadFreeSpaces(const QString& startDate, const QString& endDate)
{
    SingletonClient::getInstance()->sendMessageToServer("get_free_spaces&" + encodeValue(startDate) + "&" + encodeValue(endDate));
}

void ClientController::loadContracts()
{
    SingletonClient::getInstance()->sendMessageToServer("get_my_contracts");
}

void ClientController::loadPayments()
{
    SingletonClient::getInstance()->sendMessageToServer("get_my_payments");
}

void ClientController::loadProfile()
{
    SingletonClient::getInstance()->sendMessageToServer("get_my_profile");
}

void ClientController::updateProfile(const QString& name, const QString& address, const QString& phone,
                                     const QString& requisites, const QString& contactPerson)
{
    SingletonClient::getInstance()->sendMessageToServer(
        "update_my_profile&" + encodeValue(name) + "&" + encodeValue(address) + "&" + encodeValue(phone) + "&" + encodeValue(requisites) + "&" + encodeValue(contactPerson)
    );
}

void ClientController::changePassword(const QString& newPassword)
{
    SingletonClient::getInstance()->sendMessageToServer("change_password&" + encodeValue(newPassword));
}
