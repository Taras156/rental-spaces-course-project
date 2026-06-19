#ifndef ADMINCONTROLLER_H
#define ADMINCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

class AdminController : public QObject
{
    Q_OBJECT

public:
    static AdminController* instance();

    void loadTable(const QString& tableKey);
    void addRow(const QString& tableKey, const QStringList& values);
    void updateRow(const QString& tableKey, const QStringList& pkValues, const QStringList& values);
    void deleteRow(const QString& tableKey, const QStringList& pkValues);
    void createContract(int clientId, const QStringList& spaceIds, const QString& startDate, const QString& endDate);
    void addPayment(int contractId, int spaceId, const QString& paymentDate, const QString& amount);
    void loadFinanceReport(const QString& startDate, const QString& endDate);

private:
    explicit AdminController(QObject* parent = nullptr);
    QString encodeValue(const QString& value) const;
};

#endif // ADMINCONTROLLER_H
