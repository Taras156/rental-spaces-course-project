#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "../Models/user.h"

class AuthController : public QObject
{
    Q_OBJECT

public:
    static AuthController* instance();

    void login(const QString& login, const QString& password);

    void registerClient(const QString& login,
                        const QString& password,
                        const QString& organizationName,
                        const QString& address,
                        const QString& phone,
                        const QString& requisites,
                        const QString& contactPerson);

private:
    explicit AuthController(QObject* parent = nullptr);
    QString encodeValue(const QString& value) const;
    QString decodeValue(const QString& value) const;

private slots:
    void handleServerMessage(const QString& message);

signals:
    void loginSuccess(const User& user);
    void loginFailed(const QString& errorText);

    void registrationSuccess();
    void registrationFailed(const QString& errorText);
};

#endif // AUTHCONTROLLER_H
