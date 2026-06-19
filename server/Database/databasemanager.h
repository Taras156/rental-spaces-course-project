#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    static bool connect();
    static void disconnect();
    static QSqlDatabase database();
    static bool isConnected();
    static QString lastError();
    static QString currentDriverName();

    static bool loginUser(const QString& login,
                          const QString& password,
                          QString& role,
                          int& userId,
                          int& clientId);

    static bool registerClient(const QString& login,
                               const QString& password,
                               const QString& organizationName,
                               const QString& address,
                               const QString& phone,
                               const QString& requisites,
                               const QString& contactPerson,
                               QString& errorText);

    static QString getTable(const QString& tableKey);
    static bool addTableRow(const QString& tableKey,
                            const QStringList& values,
                            QString& errorText);
    static bool updateTableRow(const QString& tableKey,
                               const QStringList& pkValues,
                               const QStringList& values,
                               QString& errorText);
    static bool deleteTableRow(const QString& tableKey,
                               const QStringList& pkValues,
                               QString& errorText);

    static QString getFreeSpaces(const QString& startDate,
                                 const QString& endDate);
    static QString getClientContracts(int clientId);
    static QString getClientPayments(int clientId);
    static QString getClientProfile(int clientId);
    static QString getFinanceReport(const QString& startDate,
                                    const QString& endDate);
    static bool updateClientProfile(int clientId,
                                    const QString& name,
                                    const QString& address,
                                    const QString& phone,
                                    const QString& requisites,
                                    const QString& contactPerson,
                                    QString& errorText);
    static bool changePassword(int userId,
                               const QString& newPassword,
                               QString& errorText);
    static bool createContract(int clientId,
                               const QStringList& spaceIds,
                               const QString& startDate,
                               const QString& endDate,
                               QString& errorText);
    static bool addPayment(int contractId,
                           int spaceId,
                           const QString& paymentDate,
                           const QString& amount,
                           QString& errorText);

private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static bool tryOpenPsql();
    static bool tryOpenOdbc();
    static QString hashPassword(const QString& password);
    static QString encodeValue(const QString& value);

private:
    static QSqlDatabase db;
    static QString m_lastError;
    static QString m_currentDriverName;
};

#endif // DATABASEMANAGER_H
