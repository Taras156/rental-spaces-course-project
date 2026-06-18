#ifndef CLIENTCONTROLLER_H
#define CLIENTCONTROLLER_H

#include <QObject>
#include <QString>

class ClientController : public QObject
{
    Q_OBJECT

public:
    static ClientController* instance();

    void loadFreeSpaces(const QString& startDate, const QString& endDate);
    void loadContracts();
    void loadPayments();
    void loadProfile();
    void updateProfile(const QString& name, const QString& address, const QString& phone,
                       const QString& requisites, const QString& contactPerson);
    void changePassword(const QString& newPassword);

private:
    explicit ClientController(QObject* parent = nullptr);
    QString encodeValue(const QString& value) const;
};

#endif // CLIENTCONTROLLER_H
