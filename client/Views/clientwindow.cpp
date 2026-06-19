#include "clientwindow.h"
#include "../Controllers/clientcontroller.h"
#include "../Network/singletonclient.h"
#include "../Styles/thememanager.h"
#include "../Styles/themetoggleswitch.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDate>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

ClientWindow::ClientWindow(const User& user, QWidget* parent)
    : QMainWindow(parent),
      m_user(user)
{
    setupUi();
    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer,
            this, &ClientWindow::handleServerMessage);
    refreshAll();
}

void ClientWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(22, 20, 22, 20);
    mainLayout->setSpacing(14);

    QHBoxLayout* headerLayout = new QHBoxLayout();

    QVBoxLayout* titleBlock = new QVBoxLayout();
    QLabel* titleLabel = new QLabel(QString::fromUtf8("Личный кабинет арендатора"), this);
    titleLabel->setObjectName("MainTitle");

    titleBlock->addWidget(titleLabel);

    QVBoxLayout* actionsBlock = new QVBoxLayout();
    themeSwitch = new ThemeToggleSwitch(this);
    updateThemeSwitch();

    QPushButton* refreshButton = new QPushButton(QString::fromUtf8("Обновить данные"), this);
    refreshButton->setMinimumHeight(40);

    actionsBlock->addWidget(themeSwitch, 0, Qt::AlignRight);
    actionsBlock->addWidget(refreshButton);
    actionsBlock->addStretch();

    headerLayout->addLayout(titleBlock, 1);
    headerLayout->addLayout(actionsBlock);

    mainLayout->addLayout(headerLayout);

    tabs = new QTabWidget(this);

    QWidget* freeWidget = new QWidget(this);
    QVBoxLayout* freeLayout = new QVBoxLayout(freeWidget);
    freeLayout->setContentsMargins(14, 14, 14, 14);
    freeLayout->setSpacing(10);

    QLabel* freeTitle = new QLabel(QString::fromUtf8("Подбор свободных торговых площадей"), this);
    freeTitle->setObjectName("SectionTitle");
    QLabel* freeHint = new QLabel(QString::fromUtf8("Выберите период аренды, чтобы увидеть доступные торговые точки."), this);
    freeHint->setObjectName("HintLabel");
    freeHint->setWordWrap(true);

    QHBoxLayout* dateLayout = new QHBoxLayout();
    startDateEdit = new QDateEdit(QDate::currentDate(), this);
    endDateEdit = new QDateEdit(QDate::currentDate().addMonths(1), this);
    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    startDateEdit->setMinimumWidth(150);
    endDateEdit->setMinimumWidth(150);

    QPushButton* findButton = new QPushButton(QString::fromUtf8("Найти площади"), this);

    dateLayout->addWidget(new QLabel(QString::fromUtf8("Начало:"), this));
    dateLayout->addWidget(startDateEdit);
    dateLayout->addWidget(new QLabel(QString::fromUtf8("Окончание:"), this));
    dateLayout->addWidget(endDateEdit);
    dateLayout->addWidget(findButton);
    dateLayout->addStretch();

    freeSpacesTable = new QTableWidget(this);
    freeSpacesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    freeSpacesTable->verticalHeader()->setVisible(false);
    freeSpacesTable->setAlternatingRowColors(true);
    freeSpacesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    freeSpacesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    freeLayout->addWidget(freeTitle);
    freeLayout->addWidget(freeHint);
    freeLayout->addLayout(dateLayout);
    freeLayout->addWidget(freeSpacesTable);
    tabs->addTab(freeWidget, QString::fromUtf8("Доступные точки"));

    QWidget* contractsWidget = new QWidget(this);
    QVBoxLayout* contractsLayout = new QVBoxLayout(contractsWidget);
    contractsLayout->setContentsMargins(14, 14, 14, 14);
    contractsLayout->setSpacing(10);
    QLabel* contractsTitle = new QLabel(QString::fromUtf8("Мои договоры аренды"), this);
    contractsTitle->setObjectName("SectionTitle");
    contractsTable = new QTableWidget(this);
    contractsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    contractsTable->verticalHeader()->setVisible(false);
    contractsTable->setAlternatingRowColors(true);
    contractsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    contractsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contractsLayout->addWidget(contractsTitle);
    contractsLayout->addWidget(contractsTable);
    tabs->addTab(contractsWidget, QString::fromUtf8("Договоры"));

    QWidget* paymentsWidget = new QWidget(this);
    QVBoxLayout* paymentsLayout = new QVBoxLayout(paymentsWidget);
    paymentsLayout->setContentsMargins(14, 14, 14, 14);
    paymentsLayout->setSpacing(10);
    QLabel* paymentsTitle = new QLabel(QString::fromUtf8("История платежей"), this);
    paymentsTitle->setObjectName("SectionTitle");
    paymentsTable = new QTableWidget(this);
    paymentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    paymentsTable->verticalHeader()->setVisible(false);
    paymentsTable->setAlternatingRowColors(true);
    paymentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    paymentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    paymentsLayout->addWidget(paymentsTitle);
    paymentsLayout->addWidget(paymentsTable);
    tabs->addTab(paymentsWidget, QString::fromUtf8("Платежи"));

    QWidget* profileWidget = new QWidget(this);
    QVBoxLayout* profileLayout = new QVBoxLayout(profileWidget);
    profileLayout->setContentsMargins(14, 14, 14, 14);
    profileLayout->setSpacing(10);

    QLabel* profileTitle = new QLabel(QString::fromUtf8("Регистрационные данные"), this);
    profileTitle->setObjectName("SectionTitle");
    QLabel* profileHint = new QLabel(QString::fromUtf8("Измените сведения о клиенте или задайте новый пароль."), this);
    profileHint->setObjectName("HintLabel");
    profileHint->setWordWrap(true);

    QFormLayout* profileForm = new QFormLayout();
    profileForm->setHorizontalSpacing(16);
    profileForm->setVerticalSpacing(12);

    nameEdit = new QLineEdit(this);
    addressEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    requisitesEdit = new QLineEdit(this);
    contactPersonEdit = new QLineEdit(this);
    newPasswordEdit = new QLineEdit(this);
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    profileForm->addRow(QString::fromUtf8("Название:"), nameEdit);
    profileForm->addRow(QString::fromUtf8("Адрес:"), addressEdit);
    profileForm->addRow(QString::fromUtf8("Телефон:"), phoneEdit);
    profileForm->addRow(QString::fromUtf8("Реквизиты:"), requisitesEdit);
    profileForm->addRow(QString::fromUtf8("Контактное лицо:"), contactPersonEdit);
    profileForm->addRow(QString::fromUtf8("Новый пароль:"), newPasswordEdit);

    QHBoxLayout* profileButtons = new QHBoxLayout();
    QPushButton* saveProfileButton = new QPushButton(QString::fromUtf8("Сохранить данные"), this);
    QPushButton* changePasswordButton = new QPushButton(QString::fromUtf8("Сменить пароль"), this);
    profileButtons->addWidget(saveProfileButton);
    profileButtons->addWidget(changePasswordButton);
    profileButtons->addStretch();

    profileLayout->addWidget(profileTitle);
    profileLayout->addWidget(profileHint);
    profileLayout->addLayout(profileForm);
    profileLayout->addLayout(profileButtons);
    profileLayout->addStretch();
    tabs->addTab(profileWidget, QString::fromUtf8("Профиль"));

    mainLayout->addWidget(tabs);

    connect(findButton, &QPushButton::clicked, this, &ClientWindow::loadFreeSpaces);
    connect(refreshButton, &QPushButton::clicked, this, &ClientWindow::refreshAll);
    connect(saveProfileButton, &QPushButton::clicked, this, &ClientWindow::saveProfile);
    connect(changePasswordButton, &QPushButton::clicked, this, &ClientWindow::changePassword);
    connect(themeSwitch, &ThemeToggleSwitch::clicked, this, &ClientWindow::toggleTheme);

    setWindowTitle(QString::fromUtf8("Личный кабинет арендатора"));
    resize(1080, 720);
}

void ClientWindow::updateThemeSwitch()
{
    if (themeSwitch)
        themeSwitch->setChecked(ThemeManager::currentTheme() == ThemeManager::Theme9_SkyLight);
}

void ClientWindow::toggleTheme()
{
    ThemeManager::toggleTheme(qApp);
    updateThemeSwitch();
}

QString ClientWindow::decodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::fromPercentEncoding(value.toUtf8()).toUtf8());
}

void ClientWindow::refreshAll()
{
    loadFreeSpaces();
    ClientController::instance()->loadContracts();
    ClientController::instance()->loadPayments();
    ClientController::instance()->loadProfile();
}

void ClientWindow::loadFreeSpaces()
{
    ClientController::instance()->loadFreeSpaces(startDateEdit->date().toString("yyyy-MM-dd"),
                                                 endDateEdit->date().toString("yyyy-MM-dd"));
}

void ClientWindow::saveProfile()
{
    ClientController::instance()->updateProfile(nameEdit->text().trimmed(),
                                                addressEdit->text().trimmed(),
                                                phoneEdit->text().trimmed(),
                                                requisitesEdit->text().trimmed(),
                                                contactPersonEdit->text().trimmed());
}

void ClientWindow::changePassword()
{
    ClientController::instance()->changePassword(newPasswordEdit->text());
}

void ClientWindow::handleServerMessage(const QString& message)
{
    if (message.startsWith("FREE_SPACES"))
    {
        const QList<RetailSpace> spaces = parseFreeSpaces(message);
        showFreeSpaces(spaces);
        return;
    }

    if (message.startsWith("CLIENT_CONTRACTS"))
    {
        const QList<ContractInfo> contracts = parseContracts(message);
        showContracts(contracts);
        return;
    }

    if (message.startsWith("CLIENT_PAYMENTS"))
    {
        const QList<PaymentInfo> payments = parsePayments(message);
        showPayments(payments);
        return;
    }

    if (message.startsWith("CLIENT_PROFILE|"))
    {
        const Client client = parseClientProfile(message);
        showClientProfile(client);
        return;
    }

    if (message == "PROFILE_UPDATED")
    {
        QMessageBox::information(this, QString::fromUtf8("Готово"), QString::fromUtf8("Регистрационные данные сохранены."));
        ClientController::instance()->loadProfile();
        return;
    }

    if (message == "PASSWORD_CHANGED")
    {
        QMessageBox::information(this, QString::fromUtf8("Готово"), QString::fromUtf8("Пароль изменён."));
        newPasswordEdit->clear();
        return;
    }

    if (message.startsWith("ERROR&") || message.startsWith("ACCESS_DENIED&"))
    {
        QMessageBox::warning(this, QString::fromUtf8("Ошибка"), decodeValue(message.section('&', 1)));
    }
}

QList<RetailSpace> ClientWindow::parseFreeSpaces(const QString& message) const
{
    QList<RetailSpace> spaces;
    const QStringList blocks = message.split("|");

    for (int i = 1; i < blocks.size(); ++i)
    {
        const QStringList values = blocks[i].split(";");
        if (values.size() < 5)
            continue;

        const bool hasAirConditioner = decodeValue(values[3]).toLower() == "true" || decodeValue(values[3]) == QString::fromUtf8("Да");
        spaces.append(RetailSpace(
            decodeValue(values[0]).toInt(),
            decodeValue(values[1]).toInt(),
            decodeValue(values[2]).toDouble(),
            hasAirConditioner,
            decodeValue(values[4]).toDouble()
        ));
    }

    return spaces;
}

QList<ContractInfo> ClientWindow::parseContracts(const QString& message) const
{
    QList<ContractInfo> contracts;
    const QStringList blocks = message.split("|");

    for (int i = 1; i < blocks.size(); ++i)
    {
        const QStringList values = blocks[i].split(";");
        if (values.size() < 6)
            continue;

        contracts.append(ContractInfo(
            decodeValue(values[0]).toInt(),
            decodeValue(values[1]),
            decodeValue(values[2]).toInt(),
            decodeValue(values[3]),
            decodeValue(values[4]),
            decodeValue(values[5]).toDouble()
        ));
    }

    return contracts;
}

QList<PaymentInfo> ClientWindow::parsePayments(const QString& message) const
{
    QList<PaymentInfo> payments;
    const QStringList blocks = message.split("|");

    for (int i = 1; i < blocks.size(); ++i)
    {
        const QStringList values = blocks[i].split(";");
        if (values.size() < 4)
            continue;

        payments.append(PaymentInfo(
            decodeValue(values[0]).toInt(),
            decodeValue(values[1]).toInt(),
            decodeValue(values[2]),
            decodeValue(values[3]).toDouble()
        ));
    }

    return payments;
}

Client ClientWindow::parseClientProfile(const QString& message) const
{
    const QString data = message.section('|', 1);
    const QStringList values = data.split(";");

    if (values.size() < 7)
        return Client();

    return Client(
        decodeValue(values[0]).toInt(),
        decodeValue(values[1]).toInt(),
        decodeValue(values[2]),
        decodeValue(values[3]),
        decodeValue(values[4]),
        decodeValue(values[5]),
        decodeValue(values[6])
    );
}

void ClientWindow::showFreeSpaces(const QList<RetailSpace>& spaces)
{
    setupTable(freeSpacesTable,
               {QString::fromUtf8("ID точки"), QString::fromUtf8("Этаж"), QString::fromUtf8("Площадь"), QString::fromUtf8("Кондиционер"), QString::fromUtf8("Стоимость в день")},
               spaces.size());

    for (int row = 0; row < spaces.size(); ++row)
    {
        const RetailSpace& space = spaces[row];
        freeSpacesTable->setItem(row, 0, new QTableWidgetItem(QString::number(space.idSpace())));
        freeSpacesTable->setItem(row, 1, new QTableWidgetItem(QString::number(space.floorNumber())));
        freeSpacesTable->setItem(row, 2, new QTableWidgetItem(QString::number(space.area(), 'f', 2)));
        freeSpacesTable->setItem(row, 3, new QTableWidgetItem(space.hasAirConditioner() ? QString::fromUtf8("Да") : QString::fromUtf8("Нет")));
        freeSpacesTable->setItem(row, 4, new QTableWidgetItem(QString::number(space.rentPricePerDay(), 'f', 2)));
    }
}

void ClientWindow::showContracts(const QList<ContractInfo>& contracts)
{
    setupTable(contractsTable,
               {QString::fromUtf8("ID договора"), QString::fromUtf8("Дата заключения"), QString::fromUtf8("ID точки"), QString::fromUtf8("Начало"), QString::fromUtf8("Окончание"), QString::fromUtf8("Плановая сумма")},
               contracts.size());

    for (int row = 0; row < contracts.size(); ++row)
    {
        const ContractInfo& contract = contracts[row];
        contractsTable->setItem(row, 0, new QTableWidgetItem(QString::number(contract.idContract())));
        contractsTable->setItem(row, 1, new QTableWidgetItem(contract.conclusionDate()));
        contractsTable->setItem(row, 2, new QTableWidgetItem(QString::number(contract.idSpace())));
        contractsTable->setItem(row, 3, new QTableWidgetItem(contract.startDate()));
        contractsTable->setItem(row, 4, new QTableWidgetItem(contract.endDate()));
        contractsTable->setItem(row, 5, new QTableWidgetItem(QString::number(contract.plannedAmount(), 'f', 2)));
    }
}

void ClientWindow::showPayments(const QList<PaymentInfo>& payments)
{
    setupTable(paymentsTable,
               {QString::fromUtf8("ID договора"), QString::fromUtf8("ID точки"), QString::fromUtf8("Дата оплаты"), QString::fromUtf8("Сумма")},
               payments.size());

    for (int row = 0; row < payments.size(); ++row)
    {
        const PaymentInfo& payment = payments[row];
        paymentsTable->setItem(row, 0, new QTableWidgetItem(QString::number(payment.idContract())));
        paymentsTable->setItem(row, 1, new QTableWidgetItem(QString::number(payment.idSpace())));
        paymentsTable->setItem(row, 2, new QTableWidgetItem(payment.paymentDate()));
        paymentsTable->setItem(row, 3, new QTableWidgetItem(QString::number(payment.paymentAmount(), 'f', 2)));
    }
}

void ClientWindow::showClientProfile(const Client& client)
{
    if (client.idClient() == 0)
        return;

    nameEdit->setText(client.name());
    addressEdit->setText(client.address());
    phoneEdit->setText(client.phone());
    requisitesEdit->setText(client.requisites());
    contactPersonEdit->setText(client.contactPerson());
}

void ClientWindow::setupTable(QTableWidget* table, const QStringList& headers, int rowCount)
{
    table->clear();
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(rowCount);
}
