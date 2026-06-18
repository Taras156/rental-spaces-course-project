#include "clientwindow.h"
#include "../Controllers/clientcontroller.h"
#include "../Network/singletonclient.h"

#include <QAbstractItemView>
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

    centralWidget->setStyleSheet(
        "QWidget { background-color: #f4f6fb; font-family: Segoe UI; font-size: 13px; }"
        "QLabel#TitleLabel { color: #1f2a44; font-size: 26px; font-weight: 700; }"
        "QLabel#SubtitleLabel { color: #667085; font-size: 13px; }"
        "QTabWidget::pane { border: 1px solid #d0d5dd; background: white; border-radius: 12px; }"
        "QTabBar::tab { background: #e9eef8; color: #344054; padding: 10px 18px; margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px; }"
        "QTabBar::tab:selected { background: #2f66d0; color: white; font-weight: 600; }"
        "QPushButton { background-color: #2f66d0; color: white; border: none; border-radius: 8px; padding: 9px 16px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2455b5; }"
        "QLineEdit, QDateEdit { background: white; border: 1px solid #cfd7e6; border-radius: 7px; padding: 7px; }"
        "QTableWidget { background: white; border: 1px solid #d0d5dd; border-radius: 8px; gridline-color: #e4e7ec; selection-background-color: #dbeafe; }"
        "QHeaderView::section { background-color: #eef2ff; color: #1f2a44; padding: 8px; border: none; font-weight: 600; }"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    QHBoxLayout* headerLayout = new QHBoxLayout();

    QVBoxLayout* titleBlock = new QVBoxLayout();

    QLabel* titleLabel = new QLabel("Личный кабинет арендатора", this);
    titleLabel->setObjectName("TitleLabel");

    QLabel* subtitleLabel = new QLabel(
        "Просмотр доступных торговых точек, договоров, платежей и регистрационных данных.",
        this
    );
    subtitleLabel->setObjectName("SubtitleLabel");

    titleBlock->addWidget(titleLabel);
    titleBlock->addWidget(subtitleLabel);

    QPushButton* refreshButton = new QPushButton("Обновить данные", this);
    refreshButton->setMinimumHeight(38);

    headerLayout->addLayout(titleBlock);
    headerLayout->addStretch();
    headerLayout->addWidget(refreshButton);

    mainLayout->addLayout(headerLayout);

    tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);

    // =========================
    // Вкладка свободных площадей
    // =========================
    QWidget* freeWidget = new QWidget(this);
    QVBoxLayout* freeLayout = new QVBoxLayout(freeWidget);
    freeLayout->setContentsMargins(18, 18, 18, 18);
    freeLayout->setSpacing(12);

    QLabel* freeTitle = new QLabel("Подбор свободной торговой площади", this);
    freeTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2a44;");

    QLabel* freeHint = new QLabel(
        "Укажите период аренды. Система покажет только те точки, которые не заняты в выбранные даты.",
        this
    );
    freeHint->setStyleSheet("color: #667085;");

    QHBoxLayout* dateLayout = new QHBoxLayout();

    startDateEdit = new QDateEdit(QDate::currentDate(), this);
    endDateEdit = new QDateEdit(QDate::currentDate().addMonths(1), this);

    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    QPushButton* findButton = new QPushButton("Найти варианты", this);

    dateLayout->addWidget(new QLabel("С:", this));
    dateLayout->addWidget(startDateEdit);
    dateLayout->addSpacing(12);
    dateLayout->addWidget(new QLabel("По:", this));
    dateLayout->addWidget(endDateEdit);
    dateLayout->addSpacing(12);
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

    tabs->addTab(freeWidget, "Доступные точки");

    // =========================
    // Вкладка договоров
    // =========================
    QWidget* contractsWidget = new QWidget(this);
    QVBoxLayout* contractsLayout = new QVBoxLayout(contractsWidget);
    contractsLayout->setContentsMargins(18, 18, 18, 18);
    contractsLayout->setSpacing(12);

    QLabel* contractsTitle = new QLabel("Договоры аренды", this);
    contractsTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2a44;");

    QLabel* contractsHint = new QLabel(
        "В этом разделе отображаются оформленные договоры и сроки аренды торговых точек.",
        this
    );
    contractsHint->setStyleSheet("color: #667085;");

    contractsTable = new QTableWidget(this);
    contractsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    contractsTable->verticalHeader()->setVisible(false);
    contractsTable->setAlternatingRowColors(true);
    contractsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    contractsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    contractsLayout->addWidget(contractsTitle);
    contractsLayout->addWidget(contractsHint);
    contractsLayout->addWidget(contractsTable);

    tabs->addTab(contractsWidget, "Мои договоры");

    // =========================
    // Вкладка платежей
    // =========================
    QWidget* paymentsWidget = new QWidget(this);
    QVBoxLayout* paymentsLayout = new QVBoxLayout(paymentsWidget);
    paymentsLayout->setContentsMargins(18, 18, 18, 18);
    paymentsLayout->setSpacing(12);

    QLabel* paymentsTitle = new QLabel("Платежи по аренде", this);
    paymentsTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2a44;");

    QLabel* paymentsHint = new QLabel(
        "Таблица содержит историю ежемесячных платежей по договорам аренды.",
        this
    );
    paymentsHint->setStyleSheet("color: #667085;");

    paymentsTable = new QTableWidget(this);
    paymentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    paymentsTable->verticalHeader()->setVisible(false);
    paymentsTable->setAlternatingRowColors(true);
    paymentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    paymentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    paymentsLayout->addWidget(paymentsTitle);
    paymentsLayout->addWidget(paymentsHint);
    paymentsLayout->addWidget(paymentsTable);

    tabs->addTab(paymentsWidget, "Оплаты");

    // =========================
    // Вкладка профиля
    // =========================
    QWidget* profileWidget = new QWidget(this);
    QHBoxLayout* profileMainLayout = new QHBoxLayout(profileWidget);
    profileMainLayout->setContentsMargins(18, 18, 18, 18);
    profileMainLayout->setSpacing(24);

    QWidget* profileCard = new QWidget(this);
    profileCard->setStyleSheet("QWidget { background-color: white; border-radius: 14px; }");

    QVBoxLayout* profileLayout = new QVBoxLayout(profileCard);
    profileLayout->setContentsMargins(22, 22, 22, 22);
    profileLayout->setSpacing(14);

    QLabel* profileTitle = new QLabel("Данные арендатора", this);
    profileTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1f2a44;");

    QLabel* profileHint = new QLabel(
        "Здесь можно изменить сведения об организации и задать новый пароль для входа.",
        this
    );
    profileHint->setStyleSheet("color: #667085;");

    QFormLayout* profileForm = new QFormLayout();
    profileForm->setLabelAlignment(Qt::AlignRight);
    profileForm->setFormAlignment(Qt::AlignTop);
    profileForm->setHorizontalSpacing(16);
    profileForm->setVerticalSpacing(12);

    nameEdit = new QLineEdit(this);
    addressEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    requisitesEdit = new QLineEdit(this);
    contactPersonEdit = new QLineEdit(this);
    newPasswordEdit = new QLineEdit(this);
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    nameEdit->setPlaceholderText("Название организации");
    addressEdit->setPlaceholderText("Адрес организации");
    phoneEdit->setPlaceholderText("Контактный телефон");
    requisitesEdit->setPlaceholderText("Банковские реквизиты / ИНН");
    contactPersonEdit->setPlaceholderText("ФИО контактного лица");
    newPasswordEdit->setPlaceholderText("Новый пароль");

    profileForm->addRow("Организация:", nameEdit);
    profileForm->addRow("Адрес:", addressEdit);
    profileForm->addRow("Телефон:", phoneEdit);
    profileForm->addRow("Реквизиты:", requisitesEdit);
    profileForm->addRow("Контактное лицо:", contactPersonEdit);
    profileForm->addRow("Новый пароль:", newPasswordEdit);

    QHBoxLayout* profileButtons = new QHBoxLayout();

    QPushButton* saveProfileButton = new QPushButton("Сохранить данные", this);
    QPushButton* changePasswordButton = new QPushButton("Сменить пароль", this);

    changePasswordButton->setStyleSheet(
        "QPushButton { background-color: #12a150; }"
        "QPushButton:hover { background-color: #0f8a45; }"
    );

    profileButtons->addWidget(saveProfileButton);
    profileButtons->addWidget(changePasswordButton);
    profileButtons->addStretch();

    profileLayout->addWidget(profileTitle);
    profileLayout->addWidget(profileHint);
    profileLayout->addSpacing(8);
    profileLayout->addLayout(profileForm);
    profileLayout->addSpacing(8);
    profileLayout->addLayout(profileButtons);
    profileLayout->addStretch();

    QWidget* helpCard = new QWidget(this);
    helpCard->setStyleSheet("QWidget { background-color: #eef2ff; border-radius: 14px; }");

    QVBoxLayout* helpLayout = new QVBoxLayout(helpCard);
    helpLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* helpTitle = new QLabel("Памятка", this);
    helpTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #1f2a44;");

    QLabel* helpText = new QLabel(
        "Пароль должен содержать не менее 8 символов, заглавную букву, цифру и специальный символ.\n\n"
        "После редактирования сведений об организации нажмите кнопку «Сохранить данные».",
        this
    );
    helpText->setWordWrap(true);
    helpText->setStyleSheet("color: #344054;");

    helpLayout->addWidget(helpTitle);
    helpLayout->addWidget(helpText);
    helpLayout->addStretch();

    profileMainLayout->addWidget(profileCard, 3);
    profileMainLayout->addWidget(helpCard, 1);

    tabs->addTab(profileWidget, "Профиль");

    mainLayout->addWidget(tabs);

    connect(findButton, &QPushButton::clicked, this, &ClientWindow::loadFreeSpaces);
    connect(refreshButton, &QPushButton::clicked, this, &ClientWindow::refreshAll);
    connect(saveProfileButton, &QPushButton::clicked, this, &ClientWindow::saveProfile);
    connect(changePasswordButton, &QPushButton::clicked, this, &ClientWindow::changePassword);

    setWindowTitle("Личный кабинет арендатора");
    resize(1080, 720);
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
                  {"ID точки", "Этаж", "Площадь", "Кондиционер", "Стоимость в день"},
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
                  {"ID договора", "Дата заключения", "ID точки", "Начало", "Окончание", "Плановая сумма"},
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
                  {"ID договора", "ID точки", "Дата оплаты", "Сумма"},
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
        QMessageBox::information(this, "Готово", "Регистрационные данные сохранены.");
        ClientController::instance()->loadProfile();
        return;
    }

    if (message == "PASSWORD_CHANGED")
    {
        QMessageBox::information(this, "Готово", "Пароль изменён.");
        newPasswordEdit->clear();
        return;
    }

    if (message.startsWith("ERROR&") || message.startsWith("ACCESS_DENIED&"))
    {
        QMessageBox::warning(this, "Ошибка", decodeValue(message.section('&', 1)));
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
