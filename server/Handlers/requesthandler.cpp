#include "requesthandler.h"
#include "../Database/databasemanager.h"

#include <QUrl>
#include <QStringList>

static QString decodePart(const QString& value)
{
    return QString::fromUtf8(QUrl::fromPercentEncoding(value.toUtf8()).toUtf8());
}

static QString encodeValue(const QString& value)
{
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

static bool isAdmin(const QString& role)
{
    return role == "Администратор";
}

static bool isClient(const QString& role)
{
    return role == "Клиент";
}

QString RequestHandler::processRequest(const QString& request,
                                       QString& currentRole,
                                       int& currentUserId,
                                       int& currentClientId)
{
    QStringList parts = request.split("&");
    if (parts.isEmpty())
        return "ERROR&Пустой запрос";

    const QString command = parts[0];

    if (command == "login")
    {
        if (parts.size() != 3)
            return "ERROR&Неверный формат команды login";

        QString role;
        int userId = 0;
        int clientId = 0;

        bool ok = DatabaseManager::loginUser(decodePart(parts[1]), decodePart(parts[2]), role, userId, clientId);
        if (!ok)
            return "ERROR&Неверный логин или пароль";

        currentRole = role;
        currentUserId = userId;
        currentClientId = clientId;

        return "OK&" + encodeValue(role) + "&" + QString::number(userId) + "&" + QString::number(clientId);
    }


    if (command == "register_client")
    {
        if (parts.size() != 8)
            return "REGISTRATION_ERROR&" + encodeValue("Неверный формат команды регистрации");

        QString errorText;
        bool ok = DatabaseManager::registerClient(
            decodePart(parts[1]),
            decodePart(parts[2]),
            decodePart(parts[3]),
            decodePart(parts[4]),
            decodePart(parts[5]),
            decodePart(parts[6]),
            decodePart(parts[7]),
            errorText
        );

        return ok ? QString("REGISTRATION_OK")
                  : QString("REGISTRATION_ERROR&") + encodeValue(errorText);
    }

    if (currentRole.isEmpty())
        return "ACCESS_DENIED&Сначала выполните вход в систему";

    if (command == "get_table")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Полный доступ к таблицам разрешён только администратору";
        if (parts.size() != 2)
            return "ERROR&Неверный формат команды get_table";
        return DatabaseManager::getTable(parts[1]);
    }

    if (command == "add_table_row")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Добавлять записи может только администратор";
        if (parts.size() < 2)
            return "ERROR&Неверный формат команды add_table_row";

        QStringList values;
        for (int i = 2; i < parts.size(); ++i)
            values << decodePart(parts[i]);

        QString errorText;
        bool ok = DatabaseManager::addTableRow(parts[1], values, errorText);
        return ok ? "TABLE_OPERATION_OK&" + parts[1]
                  : "TABLE_OPERATION_ERROR&" + parts[1] + "&" + encodeValue(errorText);
    }

    if (command == "update_table_row")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Изменять записи может только администратор";
        if (parts.size() < 3)
            return "ERROR&Неверный формат команды update_table_row";

        const QString tableKey = parts[1];
        int pkCount = 1;
        if (tableKey == "users_roles" || tableKey == "rented_spaces")
            pkCount = 2;
        if (tableKey == "payments")
            pkCount = 3;

        if (parts.size() < 2 + pkCount)
            return "ERROR&Недостаточно значений ключа";

        QStringList pkValues;
        QStringList values;

        for (int i = 0; i < pkCount; ++i)
            pkValues << decodePart(parts[2 + i]);
        for (int i = 2 + pkCount; i < parts.size(); ++i)
            values << decodePart(parts[i]);

        QString errorText;
        bool ok = DatabaseManager::updateTableRow(tableKey, pkValues, values, errorText);
        return ok ? "TABLE_OPERATION_OK&" + tableKey
                  : "TABLE_OPERATION_ERROR&" + tableKey + "&" + encodeValue(errorText);
    }

    if (command == "delete_table_row")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Удалять записи может только администратор";
        if (parts.size() < 3)
            return "ERROR&Неверный формат команды delete_table_row";

        const QString tableKey = parts[1];
        QStringList pkValues;
        for (int i = 2; i < parts.size(); ++i)
            pkValues << decodePart(parts[i]);

        QString errorText;
        bool ok = DatabaseManager::deleteTableRow(tableKey, pkValues, errorText);
        return ok ? "TABLE_OPERATION_OK&" + tableKey
                  : "TABLE_OPERATION_ERROR&" + tableKey + "&" + encodeValue(errorText);
    }

    if (command == "get_free_spaces")
    {
        if (parts.size() != 3)
            return "ERROR&Неверный формат команды get_free_spaces";
        return DatabaseManager::getFreeSpaces(decodePart(parts[1]), decodePart(parts[2]));
    }

    if (command == "get_my_contracts")
    {
        int clientId = currentClientId;
        if (isAdmin(currentRole) && parts.size() == 2)
            clientId = parts[1].toInt();
        if (clientId <= 0)
            return "ACCESS_DENIED&Не найден клиент для текущего пользователя";
        return DatabaseManager::getClientContracts(clientId);
    }

    if (command == "get_my_payments")
    {
        int clientId = currentClientId;
        if (isAdmin(currentRole) && parts.size() == 2)
            clientId = parts[1].toInt();
        if (clientId <= 0)
            return "ACCESS_DENIED&Не найден клиент для текущего пользователя";
        return DatabaseManager::getClientPayments(clientId);
    }

    if (command == "get_my_profile")
    {
        if (!isClient(currentRole))
            return "ACCESS_DENIED&Профиль доступен только клиенту";
        return DatabaseManager::getClientProfile(currentClientId);
    }

    if (command == "update_my_profile")
    {
        if (!isClient(currentRole))
            return "ACCESS_DENIED&Изменять регистрационные данные может только клиент";
        if (parts.size() != 6)
            return "ERROR&Неверный формат команды update_my_profile";

        QString errorText;
        bool ok = DatabaseManager::updateClientProfile(currentClientId,
                                                       decodePart(parts[1]),
                                                       decodePart(parts[2]),
                                                       decodePart(parts[3]),
                                                       decodePart(parts[4]),
                                                       decodePart(parts[5]),
                                                       errorText);
        return ok ? "PROFILE_UPDATED" : "ERROR&" + encodeValue(errorText);
    }

    if (command == "change_password")
    {
        if (parts.size() != 2)
            return "ERROR&Неверный формат команды change_password";

        QString errorText;
        bool ok = DatabaseManager::changePassword(currentUserId, decodePart(parts[1]), errorText);
        return ok ? "PASSWORD_CHANGED" : "ERROR&" + encodeValue(errorText);
    }

    if (command == "create_contract")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Оформлять договоры может только администратор";
        if (parts.size() != 5)
            return "ERROR&Неверный формат команды create_contract";

        QString errorText;
        bool ok = DatabaseManager::createContract(parts[1].toInt(), parts[2].toInt(), decodePart(parts[3]), decodePart(parts[4]), errorText);
        return ok ? "CONTRACT_CREATED" : "ERROR&" + encodeValue(errorText);
    }

    if (command == "add_payment")
    {
        if (!isAdmin(currentRole))
            return "ACCESS_DENIED&Добавлять платежи может только администратор";
        if (parts.size() != 5)
            return "ERROR&Неверный формат команды add_payment";

        QString errorText;
        bool ok = DatabaseManager::addPayment(parts[1].toInt(), parts[2].toInt(), decodePart(parts[3]), decodePart(parts[4]), errorText);
        return ok ? "PAYMENT_ADDED" : "ERROR&" + encodeValue(errorText);
    }

    return "ERROR&Неизвестная команда: " + encodeValue(command);
}
