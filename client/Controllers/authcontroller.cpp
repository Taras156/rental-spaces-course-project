#include "authcontroller.h"
#include "../Network/singletonclient.h"

#include <QUrl>

AuthController* AuthController::instance()
{
    static AuthController controller;
    return &controller;
}

AuthController::AuthController(QObject* parent)
    : QObject(parent)
{
    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer,
            this, &AuthController::handleServerMessage);
}

QString AuthController::encodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

QString AuthController::decodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::fromPercentEncoding(value.toUtf8()).toUtf8());
}

void AuthController::login(const QString& login, const QString& password)
{
    SingletonClient::getInstance()->sendMessageToServer(
        "login&" + encodeValue(login) + "&" + encodeValue(password)
    );
}

void AuthController::registerClient(const QString& login,
                                    const QString& password,
                                    const QString& organizationName,
                                    const QString& address,
                                    const QString& phone,
                                    const QString& requisites,
                                    const QString& contactPerson)
{
    SingletonClient::getInstance()->sendMessageToServer(
        "register_client&" +
        encodeValue(login) + "&" +
        encodeValue(password) + "&" +
        encodeValue(organizationName) + "&" +
        encodeValue(address) + "&" +
        encodeValue(phone) + "&" +
        encodeValue(requisites) + "&" +
        encodeValue(contactPerson)
    );
}

void AuthController::handleServerMessage(const QString& message)
{
    if (message.startsWith("OK&"))
    {
        QStringList parts = message.split("&");
        if (parts.size() >= 4)
        {
            const QString role = decodeValue(parts[1]);
            const int userId = parts[2].toInt();
            const int clientId = parts[3].toInt();
            emit loginSuccess(User(userId, clientId, role));
        }
        return;
    }

    if (message == "REGISTRATION_OK")
    {
        emit registrationSuccess();
        return;
    }

    if (message.startsWith("REGISTRATION_ERROR&"))
    {
        QStringList parts = message.split("&");
        emit registrationFailed(parts.size() > 1 ? decodeValue(parts.mid(1).join("&")) : "Ошибка регистрации");
        return;
    }

    if (message.startsWith("ERROR&") || message.startsWith("ACCESS_DENIED&"))
    {
        QStringList parts = message.split("&");
        emit loginFailed(parts.size() > 1 ? decodeValue(parts.mid(1).join("&")) : "Ошибка входа");
    }
}
