#include "databasemanager.h"

#include <QCryptographicHash>
#include <QDate>
#include <QMap>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
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


QString DatabaseManager::getFinanceReport(const QString& startDate,
                                          const QString& endDate)
{
    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");

    if (!start.isValid() || !end.isValid() || start > end)
        return "ERROR&" + encodeValue("Некорректный период финансового отчета");

    QSqlQuery query(db);
    query.prepare(R"SQL(
        WITH report_months AS (
            SELECT generate_series(
                date_trunc('month', CAST(:start_date AS date)),
                date_trunc('month', CAST(:end_date AS date)),
                interval '1 month'
            )::date AS month_start
        ),
        report_rows AS (
            SELECT
                rm.month_start,
                to_char(rm.month_start, 'MM.YYYY') AS report_month,
                c.name AS client_name,
                rc.id_contract,
                rs.id_space,
                sp.floor_number,
                sp.area,
                sp.rent_price_per_day AS daily_rent,
                GREATEST(rs.start_date, rm.month_start, CAST(:start_date AS date)) AS accrual_start,
                LEAST(
                    rs.end_date,
                    (rm.month_start + interval '1 month' - interval '1 day')::date,
                    CAST(:end_date AS date)
                ) AS accrual_end
            FROM report_months rm
            JOIN rented_spaces rs
              ON rs.start_date <= LEAST((rm.month_start + interval '1 month' - interval '1 day')::date, CAST(:end_date AS date))
             AND rs.end_date >= GREATEST(rm.month_start, CAST(:start_date AS date))
            JOIN rental_contracts rc ON rc.id_contract = rs.id_contract
            JOIN clients c ON c.id_client = rc.id_client
            JOIN retail_spaces sp ON sp.id_space = rs.id_space
        )
        SELECT
            rr.report_month,
            rr.client_name,
            rr.id_contract,
            rr.id_space,
            rr.floor_number,
            rr.area,
            rr.daily_rent,
            rr.accrual_start,
            rr.accrual_end,
            (rr.accrual_end - rr.accrual_start + 1) AS rent_days,
            ((rr.accrual_end - rr.accrual_start + 1) * rr.daily_rent) AS planned_amount,
            COALESCE(SUM(p.payment_amount), 0) AS paid_amount,
            ((rr.accrual_end - rr.accrual_start + 1) * rr.daily_rent) - COALESCE(SUM(p.payment_amount), 0) AS debt,
            CASE
                WHEN COALESCE(SUM(p.payment_amount), 0) > ((rr.accrual_end - rr.accrual_start + 1) * rr.daily_rent) THEN 'Переплата'
                WHEN COALESCE(SUM(p.payment_amount), 0) = ((rr.accrual_end - rr.accrual_start + 1) * rr.daily_rent) THEN 'Оплачено'
                WHEN COALESCE(SUM(p.payment_amount), 0) = 0 THEN 'Не оплачено'
                ELSE 'Частично оплачено'
            END AS payment_status
        FROM report_rows rr
        LEFT JOIN payments p
          ON p.id_contract = rr.id_contract
         AND p.id_space = rr.id_space
         AND p.payment_date >= rr.accrual_start
         AND p.payment_date <= rr.accrual_end
        GROUP BY
            rr.month_start,
            rr.report_month,
            rr.client_name,
            rr.id_contract,
            rr.id_space,
            rr.floor_number,
            rr.area,
            rr.daily_rent,
            rr.accrual_start,
            rr.accrual_end
        ORDER BY
            rr.month_start,
            rr.client_name,
            rr.id_contract,
            rr.id_space
    )SQL");

    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);

    if (!query.exec())
    {
        qDebug() << "Finance report error:" << query.lastError().text();
        return "ERROR&" + encodeValue(query.lastError().text());
    }

    double totalPlanned = 0.0;
    double totalPaid = 0.0;
    double totalDebt = 0.0;
    double rentedArea = 0.0;
    QSet<int> contractIds;
    QSet<int> spaceIds;

    QString rows;
    while (query.next())
    {
        const int contractId = query.value("id_contract").toInt();
        const int spaceId = query.value("id_space").toInt();
        const double area = query.value("area").toDouble();
        const double dailyRent = query.value("daily_rent").toDouble();
        const int rentDays = query.value("rent_days").toInt();
        const double planned = query.value("planned_amount").toDouble();
        const double paid = query.value("paid_amount").toDouble();
        const double debt = query.value("debt").toDouble();

        totalPlanned += planned;
        totalPaid += paid;
        totalDebt += debt;
        contractIds.insert(contractId);

        if (!spaceIds.contains(spaceId))
        {
            rentedArea += area;
            spaceIds.insert(spaceId);
        }

        QString period = query.value("accrual_start").toString() + " — " + query.value("accrual_end").toString();

        QStringList values;
        values << encodeValue(query.value("report_month").toString());
        values << encodeValue(query.value("client_name").toString());
        values << QString::number(contractId);
        values << QString::number(spaceId);
        values << QString::number(query.value("floor_number").toInt());
        values << QString::number(area, 'f', 2);
        values << QString::number(dailyRent, 'f', 2);
        values << encodeValue(period);
        values << QString::number(rentDays);
        values << QString::number(planned, 'f', 2);
        values << QString::number(paid, 'f', 2);
        values << QString::number(debt, 'f', 2);
        values << encodeValue(query.value("payment_status").toString());

        rows += "|" + values.join(";");
    }

    const double percent = totalPlanned > 0.0 ? (totalPaid / totalPlanned) * 100.0 : 0.0;

    QString headers = QString::fromUtf8(
        "Месяц;Клиент;Договор;Точка;Этаж;Площадь м²;Ставка в день;Период начисления;Дней;Начислено;Оплачено;Долг;Статус"
    );

    return "FINANCE_REPORT&"
        + QString::number(totalPlanned, 'f', 2) + "&"
        + QString::number(totalPaid, 'f', 2) + "&"
        + QString::number(totalDebt, 'f', 2) + "&"
        + QString::number(percent, 'f', 2) + "&"
        + QString::number(contractIds.size()) + "&"
        + QString::number(rentedArea, 'f', 2) + "&"
        + headers + rows;
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
                                     const QStringList& spaceIds,
                                     const QString& startDate,
                                     const QString& endDate,
                                     QString& errorText)
{
    if (spaceIds.isEmpty())
    {
        errorText = "Выберите хотя бы одну торговую точку";
        return false;
    }

    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");

    if (!start.isValid() || !end.isValid() || start > end)
    {
        errorText = "Некорректный период аренды";
        return false;
    }

    if (!db.transaction())
    {
        errorText = db.lastError().text();
        return false;
    }

    QSqlQuery clientCheck(db);
    clientCheck.prepare("SELECT COUNT(*) FROM clients WHERE id_client = :client_id");
    clientCheck.bindValue(":client_id", clientId);

    if (!clientCheck.exec() || !clientCheck.next() || clientCheck.value(0).toInt() == 0)
    {
        errorText = "Клиент с указанным ID не найден";
        db.rollback();
        return false;
    }

    QSet<int> uniqueSpaceIds;
    for (const QString& value : spaceIds)
    {
        bool ok = false;
        int spaceId = value.toInt(&ok);

        if (!ok || spaceId <= 0)
        {
            errorText = "Некорректный ID торговой точки: " + value;
            db.rollback();
            return false;
        }

        if (uniqueSpaceIds.contains(spaceId))
        {
            errorText = "Одна и та же торговая точка указана несколько раз: " + QString::number(spaceId);
            db.rollback();
            return false;
        }

        uniqueSpaceIds.insert(spaceId);
    }

    for (int spaceId : uniqueSpaceIds)
    {
        QSqlQuery spaceCheck(db);
        spaceCheck.prepare("SELECT COUNT(*) FROM retail_spaces WHERE id_space = :space_id");
        spaceCheck.bindValue(":space_id", spaceId);

        if (!spaceCheck.exec() || !spaceCheck.next() || spaceCheck.value(0).toInt() == 0)
        {
            errorText = "Торговая точка с ID " + QString::number(spaceId) + " не найдена";
            db.rollback();
            return false;
        }

        QSqlQuery busyCheck(db);
        busyCheck.prepare(
            "SELECT COUNT(*) "
            "FROM rented_spaces "
            "WHERE id_space = :space_id "
            "AND daterange(start_date, end_date, '[]') && daterange(CAST(:start_date AS date), CAST(:end_date AS date), '[]')"
        );
        busyCheck.bindValue(":space_id", spaceId);
        busyCheck.bindValue(":start_date", startDate);
        busyCheck.bindValue(":end_date", endDate);

        if (!busyCheck.exec() || !busyCheck.next())
        {
            errorText = busyCheck.lastError().text();
            db.rollback();
            return false;
        }

        if (busyCheck.value(0).toInt() > 0)
        {
            errorText = "Торговая точка " + QString::number(spaceId) + " уже занята в выбранный период";
            db.rollback();
            return false;
        }
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

    for (int spaceId : uniqueSpaceIds)
    {
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
