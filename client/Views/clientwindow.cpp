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
        QList<QStringList> rows;
        QStringList blocks = message.split("|");
        for (int i = 1; i < blocks.size(); ++i)
            rows << blocks[i].split(";");
        fillTable(freeSpacesTable,
                  {QString::fromUtf8("ID точки"), QString::fromUtf8("Этаж"), QString::fromUtf8("Площадь"), QString::fromUtf8("Кондиционер"), QString::fromUtf8("Стоимость в день")},
                  rows);
        return;
    }

    if (message.startsWith("CLIENT_CONTRACTS"))
    {
        QList<QStringList> rows;
        QStringList blocks = message.split("|");
        for (int i = 1; i < blocks.size(); ++i)
            rows << blocks[i].split(";");
        fillTable(contractsTable,
                  {QString::fromUtf8("ID договора"), QString::fromUtf8("Дата заключения"), QString::fromUtf8("ID точки"), QString::fromUtf8("Начало"), QString::fromUtf8("Окончание"), QString::fromUtf8("Плановая сумма")},
                  rows);
        return;
    }

    if (message.startsWith("CLIENT_PAYMENTS"))
    {
        QList<QStringList> rows;
        QStringList blocks = message.split("|");
        for (int i = 1; i < blocks.size(); ++i)
            rows << blocks[i].split(";");
        fillTable(paymentsTable,
                  {QString::fromUtf8("ID договора"), QString::fromUtf8("ID точки"), QString::fromUtf8("Дата оплаты"), QString::fromUtf8("Сумма")},
                  rows);
        return;
    }

    if (message.startsWith("CLIENT_PROFILE|"))
    {
        QString data = message.section('|', 1);
        QStringList values = data.split(";");
        if (values.size() >= 7)
        {
            nameEdit->setText(decodeValue(values[2]));
            addressEdit->setText(decodeValue(values[3]));
            phoneEdit->setText(decodeValue(values[4]));
            requisitesEdit->setText(decodeValue(values[5]));
            contactPersonEdit->setText(decodeValue(values[6]));
        }
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

void ClientWindow::fillTable(QTableWidget* table, const QStringList& headers, const QList<QStringList>& rows)
{
    table->clear();
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(rows.size());

    for (int row = 0; row < rows.size(); ++row)
    {
        for (int col = 0; col < headers.size(); ++col)
        {
            QString value = col < rows[row].size() ? decodeValue(rows[row][col]) : QString();
            table->setItem(row, col, new QTableWidgetItem(value));
        }
    }
}
