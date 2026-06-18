#include "admincontroller.h"
#include "../Network/singletonclient.h"

#include <QUrl>

AdminController* AdminController::instance()
{
    static AdminController controller;
    return &controller;
}

AdminController::AdminController(QObject* parent)
    : QObject(parent)
{
}

QString AdminController::encodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

void AdminController::loadTable(const QString& tableKey)
{
    SingletonClient::getInstance()->sendMessageToServer("get_table&" + tableKey);
}

void AdminController::addRow(const QString& tableKey, const QStringList& values)
{
    QString request = "add_table_row&" + tableKey;
    for (const QString& value : values)
        request += "&" + encodeValue(value);
    SingletonClient::getInstance()->sendMessageToServer(request);
}

void AdminController::updateRow(const QString& tableKey, const QStringList& pkValues, const QStringList& values)
{
    QString request = "update_table_row&" + tableKey;
    for (const QString& value : pkValues)
        request += "&" + encodeValue(value);
    for (const QString& value : values)
        request += "&" + encodeValue(value);
    SingletonClient::getInstance()->sendMessageToServer(request);
}

void AdminController::deleteRow(const QString& tableKey, const QStringList& pkValues)
{
    QString request = "delete_table_row&" + tableKey;
    for (const QString& value : pkValues)
        request += "&" + encodeValue(value);
    SingletonClient::getInstance()->sendMessageToServer(request);
}

void AdminController::createContract(int clientId, int spaceId, const QString& startDate, const QString& endDate)
{
    SingletonClient::getInstance()->sendMessageToServer(
        "create_contract&" + QString::number(clientId) + "&" + QString::number(spaceId) + "&" + encodeValue(startDate) + "&" + encodeValue(endDate)
    );
}

void AdminController::addPayment(int contractId, int spaceId, const QString& paymentDate, const QString& amount)
{
    SingletonClient::getInstance()->sendMessageToServer(
        "add_payment&" + QString::number(contractId) + "&" + QString::number(spaceId) + "&" + encodeValue(paymentDate) + "&" + encodeValue(amount)
    );
}
