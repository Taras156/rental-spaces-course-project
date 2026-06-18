#include "admintablewindow.h"
#include "../Controllers/admincontroller.h"
#include "../Network/singletonclient.h"

#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

AdminTableWindow::AdminTableWindow(const QString& tableKey,
                                   const QString& windowTitle,
                                   QWidget* parent)
    : QMainWindow(parent),
      m_tableKey(tableKey),
      m_title(windowTitle),
      m_pkCount(1),
      table(nullptr)
{
    setupUi();
    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer,
            this, &AdminTableWindow::handleServerMessage);
    loadData();
}

void AdminTableWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QLabel* titleLabel = new QLabel(m_title, this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    table = new QTableWidget(this);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    mainLayout->addWidget(table);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    refreshButton = new QPushButton("Обновить", this);
    addButton = new QPushButton("Добавить", this);
    editButton = new QPushButton("Изменить", this);
    deleteButton = new QPushButton("Удалить", this);

    buttonsLayout->addWidget(refreshButton);
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);

    connect(refreshButton, &QPushButton::clicked, this, &AdminTableWindow::loadData);
    connect(addButton, &QPushButton::clicked, this, &AdminTableWindow::addRecord);
    connect(editButton, &QPushButton::clicked, this, &AdminTableWindow::editRecord);
    connect(deleteButton, &QPushButton::clicked, this, &AdminTableWindow::deleteRecord);

    setWindowTitle(m_title);
    resize(1100, 650);
}

QString AdminTableWindow::decodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::fromPercentEncoding(value.toUtf8()).toUtf8());
}

void AdminTableWindow::loadData()
{
    AdminController::instance()->loadTable(m_tableKey);
}

void AdminTableWindow::addRecord()
{
    if (m_addColumns.isEmpty())
    {
        QMessageBox::information(this, "Добавление", "Для этой таблицы добавление через форму не настроено.");
        return;
    }

    QStringList values = showEditDialog("Добавить запись", m_addColumns, false);
    if (values.isEmpty())
        return;

    AdminController::instance()->addRow(m_tableKey, values);
}

void AdminTableWindow::editRecord()
{
    if (table->currentRow() < 0)
    {
        QMessageBox::warning(this, "Изменение", "Сначала выберите строку.");
        return;
    }

    if (m_updateColumns.isEmpty())
    {
        QMessageBox::information(this, "Изменение", "У этой таблицы нет изменяемых полей.");
        return;
    }

    QStringList values = showEditDialog("Изменить запись", m_updateColumns, true);
    if (values.isEmpty())
        return;

    QStringList pkValues;
    for (int i = 0; i < m_pkCount && i < m_columns.size(); ++i)
        pkValues << valueFromSelectedRow(m_columns[i]);

    AdminController::instance()->updateRow(m_tableKey, pkValues, values);
}

void AdminTableWindow::deleteRecord()
{
    if (table->currentRow() < 0)
    {
        QMessageBox::warning(this, "Удаление", "Сначала выберите строку.");
        return;
    }

    if (QMessageBox::question(this, "Удаление", "Удалить выбранную запись?") != QMessageBox::Yes)
        return;

    QStringList pkValues;
    for (int i = 0; i < m_pkCount && i < m_columns.size(); ++i)
        pkValues << valueFromSelectedRow(m_columns[i]);

    AdminController::instance()->deleteRow(m_tableKey, pkValues);
}

void AdminTableWindow::handleServerMessage(const QString& message)
{
    if (message.startsWith("TABLE_OPERATION_OK&" + m_tableKey))
    {
        loadData();
        return;
    }

    if (message.startsWith("TABLE_OPERATION_ERROR&" + m_tableKey))
    {
        QStringList parts = message.split("&");
        QString errorText = parts.size() >= 3 ? decodeValue(parts.mid(2).join("&")) : "Не удалось выполнить операцию.";
        QMessageBox::warning(this, "Ошибка", errorText);
        return;
    }

    if (!message.startsWith("TABLE_DATA&" + m_tableKey + "&"))
        return;

    QStringList blocks = message.split("|");
    if (blocks.isEmpty())
        return;

    QStringList header = blocks[0].split("&");
    if (header.size() < 7)
        return;

    m_title = decodeValue(header[2]);
    m_pkCount = header[3].toInt();
    m_columns = header[4].split(";", Qt::SkipEmptyParts);
    m_addColumns = header[5].split(";", Qt::SkipEmptyParts);
    m_updateColumns = header[6].split(";", Qt::SkipEmptyParts);

    table->clear();
    table->setColumnCount(m_columns.size());
    table->setHorizontalHeaderLabels(displayNames(m_columns));
    table->setRowCount(blocks.size() - 1);

    for (int row = 1; row < blocks.size(); ++row)
    {
        QStringList values = blocks[row].split(";");
        for (int col = 0; col < m_columns.size(); ++col)
        {
            QString value = col < values.size() ? decodeValue(values[col]) : "";
            table->setItem(row - 1, col, new QTableWidgetItem(value));
        }
    }
}

QStringList AdminTableWindow::showEditDialog(const QString& title, const QStringList& columns, bool fillCurrentValues)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);

    QFormLayout* formLayout = new QFormLayout(&dialog);
    QMap<QString, QLineEdit*> editors;

    for (const QString& column : columns)
    {
        QLineEdit* editor = new QLineEdit(&dialog);
        if (fillCurrentValues)
            editor->setText(valueFromSelectedRow(column));
        editors.insert(column, editor);
        formLayout->addRow(displayName(column) + ":", editor);
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return QStringList();

    QStringList values;
    for (const QString& column : columns)
        values << editors[column]->text().trimmed();

    return values;
}

QString AdminTableWindow::valueFromSelectedRow(const QString& columnName) const
{
    int columnIndex = m_columns.indexOf(columnName);
    if (columnIndex < 0 || table->currentRow() < 0 || !table->item(table->currentRow(), columnIndex))
        return QString();
    return table->item(table->currentRow(), columnIndex)->text();
}

QString AdminTableWindow::displayName(const QString& columnName) const
{
    static const QMap<QString, QString> names = {
        {"id_role", "ID роли"},
        {"id_user", "ID пользователя"},
        {"id_client", "ID клиента"},
        {"id_space", "ID точки"},
        {"id_contract", "ID договора"},
        {"name", "Название"},
        {"description", "Описание"},
        {"login", "Логин"},
        {"password_hash", "Хэш пароля"},
        {"address", "Адрес"},
        {"phone", "Телефон"},
        {"requisites", "Реквизиты"},
        {"contact_person", "Контактное лицо"},
        {"rent_price_per_day", "Стоимость аренды в день"},
        {"has_air_conditioner", "Кондиционер"},
        {"area", "Площадь"},
        {"floor_number", "Этаж"},
        {"is_available", "Свободная"},
        {"conclusion_date", "Дата заключения"},
        {"start_date", "Дата начала"},
        {"end_date", "Дата окончания"},
        {"payment_date", "Дата оплаты"},
        {"payment_amount", "Сумма платежа"}
    };
    return names.value(columnName, columnName);
}

QStringList AdminTableWindow::displayNames(const QStringList& columnNames) const
{
    QStringList result;
    for (const QString& column : columnNames)
        result << displayName(column);
    return result;
}
