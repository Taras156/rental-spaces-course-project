#include "databasemanager.h"

#include <QCryptographicHash>
#include <QDate>
#include <QMap>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUrl>
#include <QVariant>
#include <QDebug>

QSqlDatabase DatabaseManager::db;
QString DatabaseManager::m_lastError;
QString DatabaseManager::m_currentDriverName;

namespace
{
    const QString PG_HOST = "localhost";
    const int PG_PORT = 5432;
    const QString PG_DATABASE = "rental_spaces_db";
    const QString PG_USER = "postgres";
    const QString PG_PASSWORD = "Taras156";
    const QString ODBC_DSN = "RentalSpacesDB";

    struct TableConfig
    {
        QString tableName;
        QString title;
        QStringList columns;
        QStringList pkColumns;
        QStringList addColumns;
        QStringList updateColumns;
        QString orderBy;
    };

    QMap<QString, TableConfig> tableConfigs()
    {
        QMap<QString, TableConfig> map;
        map.insert("roles", {"roles", "Роли", {"id_role", "name", "description"}, {"id_role"}, {"name", "description"}, {"name", "description"}, "id_role"});
        map.insert("users", {"users", "Пользователи", {"id_user", "login", "password_hash"}, {"id_user"}, {"login", "password_hash"}, {"login", "password_hash"}, "id_user"});
        map.insert("users_roles", {"users_roles", "Пользователи и роли", {"id_user", "id_role"}, {"id_user", "id_role"}, {"id_user", "id_role"}, {}, "id_user, id_role"});
        map.insert("clients", {"clients", "Клиенты", {"id_client", "id_user", "name", "address", "phone", "requisites", "contact_person"}, {"id_client"}, {"id_user", "name", "address", "phone", "requisites", "contact_person"}, {"id_user", "name", "address", "phone", "requisites", "contact_person"}, "id_client"});
        map.insert("retail_spaces", {"retail_spaces", "Торговые точки", {"id_space", "rent_price_per_day", "has_air_conditioner", "area", "floor_number", "is_available"}, {"id_space"}, {"rent_price_per_day", "has_air_conditioner", "area", "floor_number", "is_available"}, {"rent_price_per_day", "has_air_conditioner", "area", "floor_number", "is_available"}, "id_space"});
        map.insert("rental_contracts", {"rental_contracts", "Договоры аренды", {"id_contract", "id_client", "conclusion_date"}, {"id_contract"}, {"id_client", "conclusion_date"}, {"id_client", "conclusion_date"}, "id_contract"});
        map.insert("rented_spaces", {"rented_spaces", "Арендуемые торговые точки", {"id_contract", "id_space", "start_date", "end_date"}, {"id_contract", "id_space"}, {"id_contract", "id_space", "start_date", "end_date"}, {"start_date", "end_date"}, "id_contract, id_space"});
        map.insert("payments", {"payments", "Платежи", {"id_contract", "id_space", "payment_date", "payment_amount"}, {"id_contract", "id_space", "payment_date"}, {"id_contract", "id_space", "payment_date", "payment_amount"}, {"payment_amount"}, "id_contract, id_space, payment_date"});
        return map;
    }

    QString sqlList(const QStringList& list)
    {
        return list.join(", ");
    }

    QString makeWhereByPk(const QStringList& pkColumns)
    {
        QStringList parts;
        for (int i = 0; i < pkColumns.size(); ++i)
            parts << pkColumns[i] + " = :pk" + QString::number(i);
        return parts.join(" AND ");
    }
}

QString DatabaseManager::hashPassword(const QString& password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString DatabaseManager::encodeValue(const QString& value)
{
    return QString::fromUtf8(QUrl::toPercentEncoding(value));
}

bool DatabaseManager::connect()
{
    if (db.isValid() && db.isOpen())
        return true;

    QStringList errors;

    if (tryOpenPsql())
        return true;

    errors << "QPSQL: " + m_lastError;

    m_lastError = errors.join("\n");
    qDebug() << "Database connection error:" << m_lastError;
    return false;
}
bool DatabaseManager::tryOpenPsql()
{
    if (!QSqlDatabase::drivers().contains("QPSQL"))
    {
        m_lastError = "Драйвер QPSQL отсутствует.";
        return false;
    }

    db = QSqlDatabase::addDatabase("QPSQL", "server_psql_connection");
    db.setHostName(PG_HOST);
    db.setPort(PG_PORT);
    db.setDatabaseName(PG_DATABASE);
    db.setUserName(PG_USER);
    db.setPassword(PG_PASSWORD);

    if (!db.open())
    {
        m_lastError = db.lastError().text();
        db.close();
        return false;
    }

    m_currentDriverName = "QPSQL";
    m_lastError.clear();
    qDebug() << "Database connected through QPSQL";
    return true;
}

bool DatabaseManager::tryOpenOdbc()
{
    m_lastError = "QODBC не используется. Подключение выполняется через QPSQL.";
    return false;
}
void DatabaseManager::disconnect()
{
    if (db.isValid() && db.isOpen())
        db.close();
}

QSqlDatabase DatabaseManager::database()
{
    return db;
}

bool DatabaseManager::isConnected()
{
    return db.isValid() && db.isOpen();
}

QString DatabaseManager::lastError()
{
    return m_lastError;
}

QString DatabaseManager::currentDriverName()
{
    return m_currentDriverName;
}

bool DatabaseManager::loginUser(const QString& login,
                                const QString& password,
                                QString& role,
                                int& userId,
                                int& clientId)
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT u.id_user, COALESCE(c.id_client, 0) AS id_client, r.name AS role_name "
        "FROM users u "
        "JOIN users_roles ur ON ur.id_user = u.id_user "
        "JOIN roles r ON r.id_role = ur.id_role "
        "LEFT JOIN clients c ON c.id_user = u.id_user "
        "WHERE u.login = :login AND u.password_hash = :password_hash "
        "ORDER BY r.name"
    );
    query.bindValue(":login", login);
    query.bindValue(":password_hash", hashPassword(password));

    if (!query.exec())
    {
        qDebug() << "Login error:" << query.lastError().text();
        return false;
    }

    role.clear();
    userId = 0;
    clientId = 0;

    while (query.next())
    {
        userId = query.value("id_user").toInt();
        clientId = query.value("id_client").toInt();
        const QString currentRole = query.value("role_name").toString();

        if (role.isEmpty())
            role = currentRole;
        if (currentRole == "Администратор")
            role = currentRole;
    }

    return userId > 0;
}

bool DatabaseManager::registerClient(const QString& login,
                                     const QString& password,
                                     const QString& organizationName,
                                     const QString& address,
                                     const QString& phone,
                                     const QString& requisites,
                                     const QString& contactPerson,
                                     QString& errorText)
{
    if (login.trimmed().isEmpty() ||
        password.isEmpty() ||
        organizationName.trimmed().isEmpty() ||
        address.trimmed().isEmpty() ||
        phone.trimmed().isEmpty() ||
        requisites.trimmed().isEmpty() ||
        contactPerson.trimmed().isEmpty())
    {
        errorText = "Заполните все поля регистрации";
        return false;
    }

    QRegularExpression regex(R"(^(?=.*[A-Z])(?=.*\d)(?=.*[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]).{8,}$)");
    if (!regex.match(password).hasMatch())
    {
        errorText = "Пароль должен быть не короче 8 символов и содержать цифру, заглавную букву и специальный символ";
        return false;
    }

    if (!db.transaction())
    {
        errorText = db.lastError().text();
        return false;
    }

    QSqlQuery userQuery(db);
    userQuery.prepare(
        "INSERT INTO users (login, password_hash) "
        "VALUES (:login, :password_hash) "
        "RETURNING id_user"
    );
    userQuery.bindValue(":login", login.trimmed());
    userQuery.bindValue(":password_hash", hashPassword(password));

    if (!userQuery.exec() || !userQuery.next())
    {
        errorText = userQuery.lastError().text();
        if (errorText.contains("users_login_key"))
            errorText = "Пользователь с таким логином уже существует";
        db.rollback();
        return false;
    }

    const int userId = userQuery.value(0).toInt();

    QSqlQuery roleQuery(db);
    roleQuery.prepare(
        "INSERT INTO users_roles (id_user, id_role) "
        "SELECT :user_id, id_role "
        "FROM roles "
        "WHERE name = 'Клиент'"
    );
    roleQuery.bindValue(":user_id", userId);

    if (!roleQuery.exec() || roleQuery.numRowsAffected() != 1)
    {
        errorText = "Не удалось назначить роль клиента. Проверьте наличие роли «Клиент» в таблице roles";
        db.rollback();
        return false;
    }

    QSqlQuery clientQuery(db);
    clientQuery.prepare(
        "INSERT INTO clients (id_user, name, address, phone, requisites, contact_person) "
        "VALUES (:user_id, :name, :address, :phone, :requisites, :contact_person)"
    );
    clientQuery.bindValue(":user_id", userId);
    clientQuery.bindValue(":name", organizationName.trimmed());
    clientQuery.bindValue(":address", address.trimmed());
    clientQuery.bindValue(":phone", phone.trimmed());
    clientQuery.bindValue(":requisites", requisites.trimmed());
    clientQuery.bindValue(":contact_person", contactPerson.trimmed());

    if (!clientQuery.exec())
    {
        errorText = clientQuery.lastError().text();
        if (errorText.contains("clients_phone_key"))
            errorText = "Клиент с таким телефоном уже существует";
        else if (errorText.contains("clients_requisites_key"))
            errorText = "Клиент с такими реквизитами уже существует";
        else if (errorText.contains("clients_name_address_key"))
            errorText = "Клиент с таким названием и адресом уже существует";

        db.rollback();
        return false;
    }

    if (!db.commit())
    {
        errorText = db.lastError().text();
        return false;
    }

    return true;
}


QString DatabaseManager::getTable(const QString& tableKey)
{
    const auto configs = tableConfigs();
    if (!configs.contains(tableKey))
        return "TABLE_OPERATION_ERROR&" + tableKey + "&Неизвестная таблица";

    const TableConfig cfg = configs.value(tableKey);
    QSqlQuery query(db);
    const QString sql = "SELECT " + sqlList(cfg.columns) + " FROM " + cfg.tableName + " ORDER BY " + cfg.orderBy;

    if (!query.exec(sql))
    {
        qDebug() << "getTable error:" << query.lastError().text();
        return "TABLE_OPERATION_ERROR&" + tableKey + "&" + encodeValue(query.lastError().text());
    }

    QString result = "TABLE_DATA&" + tableKey + "&" + encodeValue(cfg.title) + "&" + QString::number(cfg.pkColumns.size()) + "&"
        + cfg.columns.join(";") + "&" + cfg.addColumns.join(";") + "&" + cfg.updateColumns.join(";");

    while (query.next())
    {
        QStringList values;
        for (const QString& column : cfg.columns)
        {
            QVariant value = query.value(column);
            if (value.type() == QVariant::Bool)
                values << (value.toBool() ? "true" : "false");
            else
                values << encodeValue(value.toString());
        }
        result += "|" + values.join(";");
    }

    return result;
}

bool DatabaseManager::addTableRow(const QString& tableKey,
                                  const QStringList& values,
                                  QString& errorText)
{
    const auto configs = tableConfigs();
    if (!configs.contains(tableKey))
    {
        errorText = "Неизвестная таблица";
        return false;
    }

    const TableConfig cfg = configs.value(tableKey);
    if (values.size() != cfg.addColumns.size())
    {
        errorText = "Неверное количество значений";
        return false;
    }

    QStringList placeholders;
    for (int i = 0; i < cfg.addColumns.size(); ++i)
        placeholders << ":v" + QString::number(i);

    QSqlQuery query(db);
    query.prepare("INSERT INTO " + cfg.tableName + " (" + sqlList(cfg.addColumns) + ") VALUES (" + placeholders.join(", ") + ")");
    for (int i = 0; i < values.size(); ++i)
        query.bindValue(":v" + QString::number(i), values[i]);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateTableRow(const QString& tableKey,
                                     const QStringList& pkValues,
                                     const QStringList& values,
                                     QString& errorText)
{
    const auto configs = tableConfigs();
    if (!configs.contains(tableKey))
    {
        errorText = "Неизвестная таблица";
        return false;
    }

    const TableConfig cfg = configs.value(tableKey);
    if (pkValues.size() != cfg.pkColumns.size() || values.size() != cfg.updateColumns.size())
    {
        errorText = "Неверное количество значений";
        return false;
    }

    if (cfg.updateColumns.isEmpty())
    {
        errorText = "Для этой таблицы изменение не настроено";
        return false;
    }

    QStringList setParts;
    for (int i = 0; i < cfg.updateColumns.size(); ++i)
        setParts << cfg.updateColumns[i] + " = :v" + QString::number(i);

    QSqlQuery query(db);
    query.prepare("UPDATE " + cfg.tableName + " SET " + setParts.join(", ") + " WHERE " + makeWhereByPk(cfg.pkColumns));

    for (int i = 0; i < values.size(); ++i)
        query.bindValue(":v" + QString::number(i), values[i]);
    for (int i = 0; i < pkValues.size(); ++i)
        query.bindValue(":pk" + QString::number(i), pkValues[i]);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteTableRow(const QString& tableKey,
                                     const QStringList& pkValues,
                                     QString& errorText)
{
    const auto configs = tableConfigs();
    if (!configs.contains(tableKey))
    {
        errorText = "Неизвестная таблица";
        return false;
    }

    const TableConfig cfg = configs.value(tableKey);
    if (pkValues.size() != cfg.pkColumns.size())
    {
        errorText = "Неверное количество значений ключа";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM " + cfg.tableName + " WHERE " + makeWhereByPk(cfg.pkColumns));
    for (int i = 0; i < pkValues.size(); ++i)
        query.bindValue(":pk" + QString::number(i), pkValues[i]);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }

    return true;
}

QString DatabaseManager::getFreeSpaces(const QString& startDate,
                                       const QString& endDate)
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT s.id_space, s.floor_number, s.area, s.has_air_conditioner, s.rent_price_per_day "
        "FROM retail_spaces s "
        "WHERE NOT EXISTS ( "
        "    SELECT 1 FROM rented_spaces r "
        "    WHERE r.id_space = s.id_space "
        "      AND daterange(r.start_date, r.end_date, '[]') && daterange(CAST(:start_date AS date), CAST(:end_date AS date), '[]') "
        ") "
        "ORDER BY s.id_space"
    );
    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);

    if (!query.exec())
        return "ERROR&" + encodeValue(query.lastError().text());

    QString result = "FREE_SPACES";
    while (query.next())
    {
        result += "|" + query.value(0).toString()
            + ";" + query.value(1).toString()
            + ";" + query.value(2).toString()
            + ";" + (query.value(3).toBool() ? "Да" : "Нет")
            + ";" + query.value(4).toString();
    }
    return result;
}

QString DatabaseManager::getClientContracts(int clientId)
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT rc.id_contract, rc.conclusion_date, rs.id_space, rs.start_date, rs.end_date, "
        "       (rs.end_date - rs.start_date + 1) * s.rent_price_per_day AS planned_amount "
        "FROM rental_contracts rc "
        "JOIN rented_spaces rs ON rs.id_contract = rc.id_contract "
        "JOIN retail_spaces s ON s.id_space = rs.id_space "
        "WHERE rc.id_client = :client_id "
        "ORDER BY rc.id_contract, rs.id_space"
    );
    query.bindValue(":client_id", clientId);

    if (!query.exec())
        return "ERROR&" + encodeValue(query.lastError().text());

    QString result = "CLIENT_CONTRACTS";
    while (query.next())
    {
        result += "|" + query.value(0).toString()
            + ";" + query.value(1).toString()
            + ";" + query.value(2).toString()
            + ";" + query.value(3).toString()
            + ";" + query.value(4).toString()
            + ";" + QString::number(query.value(5).toDouble(), 'f', 2);
    }
    return result;
}

QString DatabaseManager::getClientPayments(int clientId)
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT p.id_contract, p.id_space, p.payment_date, p.payment_amount "
        "FROM payments p "
        "JOIN rental_contracts rc ON rc.id_contract = p.id_contract "
        "WHERE rc.id_client = :client_id "
        "ORDER BY p.payment_date, p.id_contract, p.id_space"
    );
    query.bindValue(":client_id", clientId);

    if (!query.exec())
        return "ERROR&" + encodeValue(query.lastError().text());

    QString result = "CLIENT_PAYMENTS";
    while (query.next())
    {
        result += "|" + query.value(0).toString()
            + ";" + query.value(1).toString()
            + ";" + query.value(2).toString()
            + ";" + QString::number(query.value(3).toDouble(), 'f', 2);
    }
    return result;
}

QString DatabaseManager::getClientProfile(int clientId)
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT id_client, id_user, name, address, phone, requisites, contact_person "
        "FROM clients WHERE id_client = :client_id"
    );
    query.bindValue(":client_id", clientId);

    if (!query.exec())
        return "ERROR&" + encodeValue(query.lastError().text());

    if (!query.next())
        return "ERROR&Клиент не найден";

    return "CLIENT_PROFILE|"
        + query.value(0).toString() + ";"
        + query.value(1).toString() + ";"
        + encodeValue(query.value(2).toString()) + ";"
        + encodeValue(query.value(3).toString()) + ";"
        + encodeValue(query.value(4).toString()) + ";"
        + encodeValue(query.value(5).toString()) + ";"
        + encodeValue(query.value(6).toString());
}

bool DatabaseManager::updateClientProfile(int clientId,
                                          const QString& name,
                                          const QString& address,
                                          const QString& phone,
                                          const QString& requisites,
                                          const QString& contactPerson,
                                          QString& errorText)
{
    QSqlQuery query(db);
    query.prepare(
        "UPDATE clients "
        "SET name = :name, address = :address, phone = :phone, requisites = :requisites, contact_person = :contact_person "
        "WHERE id_client = :client_id"
    );
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":phone", phone);
    query.bindValue(":requisites", requisites);
    query.bindValue(":contact_person", contactPerson);
    query.bindValue(":client_id", clientId);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::changePassword(int userId,
                                     const QString& newPassword,
                                     QString& errorText)
{
    QRegularExpression regex(R"(^(?=.*[A-Z])(?=.*\d)(?=.*[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]).{8,}$)");
    if (!regex.match(newPassword).hasMatch())
    {
        errorText = "Пароль должен быть не короче 8 символов и содержать цифру, заглавную букву и специальный символ";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE users SET password_hash = :password_hash WHERE id_user = :user_id");
    query.bindValue(":password_hash", hashPassword(newPassword));
    query.bindValue(":user_id", userId);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::createContract(int clientId,
                                     int spaceId,
                                     const QString& startDate,
                                     const QString& endDate,
                                     QString& errorText)
{
    if (!db.transaction())
    {
        errorText = db.lastError().text();
        return false;
    }

    QSqlQuery contractQuery(db);
    contractQuery.prepare(
        "INSERT INTO rental_contracts (id_client, conclusion_date) "
        "VALUES (:client_id, CURRENT_DATE) RETURNING id_contract"
    );
    contractQuery.bindValue(":client_id", clientId);

    if (!contractQuery.exec() || !contractQuery.next())
    {
        errorText = contractQuery.lastError().text();
        db.rollback();
        return false;
    }

    int contractId = contractQuery.value(0).toInt();

    QSqlQuery spaceQuery(db);
    spaceQuery.prepare(
        "INSERT INTO rented_spaces (id_contract, id_space, start_date, end_date) "
        "VALUES (:contract_id, :space_id, :start_date, :end_date)"
    );
    spaceQuery.bindValue(":contract_id", contractId);
    spaceQuery.bindValue(":space_id", spaceId);
    spaceQuery.bindValue(":start_date", startDate);
    spaceQuery.bindValue(":end_date", endDate);

    if (!spaceQuery.exec())
    {
        errorText = spaceQuery.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit())
    {
        errorText = db.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addPayment(int contractId,
                                 int spaceId,
                                 const QString& paymentDate,
                                 const QString& amount,
                                 QString& errorText)
{
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO payments (id_contract, id_space, payment_date, payment_amount) "
        "VALUES (:contract_id, :space_id, :payment_date, :payment_amount)"
    );
    query.bindValue(":contract_id", contractId);
    query.bindValue(":space_id", spaceId);
    query.bindValue(":payment_date", paymentDate);
    query.bindValue(":payment_amount", amount);

    if (!query.exec())
    {
        errorText = query.lastError().text();
        return false;
    }
    return true;
}
