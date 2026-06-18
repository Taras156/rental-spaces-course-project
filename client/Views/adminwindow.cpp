#include "adminwindow.h"
#include "admintablewindow.h"
#include "../Controllers/admincontroller.h"
#include "../Network/singletonclient.h"

#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

AdminWindow::AdminWindow(const User& user, QWidget* parent)
    : QMainWindow(parent),
      m_user(user)
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    centralWidget->setStyleSheet(
        "QWidget { background-color: #f7f9fc; color: #162033; font-family: Segoe UI; font-size: 14px; }"
        "QFrame#TopBar { background-color: #ffffff; border: 1px solid #e4e7ec; border-radius: 18px; }"
        "QFrame#InfoCard { background-color: #0f3d7a; border-radius: 18px; }"
        "QLabel#MainTitle { color: #111827; font-size: 28px; font-weight: 800; }"
        "QLabel#Subtitle { color: #667085; font-size: 14px; }"
        "QLabel#CardTitle { color: white; font-size: 18px; font-weight: 700; }"
        "QLabel#CardText { color: #dbeafe; font-size: 13px; }"
        "QGroupBox { background-color: #ffffff; border: 1px solid #e4e7ec; border-radius: 16px; margin-top: 18px; padding: 18px 14px 14px 14px; font-weight: 700; color: #344054; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 18px; padding: 0 8px; background-color: #ffffff; color: #1d4ed8; }"
        "QPushButton { background-color: #ffffff; color: #1f2937; border: 1px solid #d0d5dd; border-radius: 14px; padding: 14px 16px; text-align: left; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background-color: #eef4ff; border: 1px solid #2f66d0; color: #1d4ed8; }"
        "QPushButton#PrimaryButton { background-color: #2563eb; color: white; border: none; }"
        "QPushButton#PrimaryButton:hover { background-color: #1d4ed8; }"
        "QPushButton#GreenButton { background-color: #12a150; color: white; border: none; }"
        "QPushButton#GreenButton:hover { background-color: #0f8a45; }"
        "QScrollArea { border: none; background-color: transparent; }"
        "QLineEdit, QSpinBox, QDateEdit { background-color: #ffffff; border: 1px solid #cfd7e6; border-radius: 8px; padding: 7px; }"
    );

    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(22, 18, 22, 18);
    rootLayout->setSpacing(18);

    QFrame* topBar = new QFrame(this);
    topBar->setObjectName("TopBar");
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(22, 18, 22, 18);
    topLayout->setSpacing(20);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(6);

    QLabel* titleLabel = new QLabel("Административный центр", this);
    titleLabel->setObjectName("MainTitle");

    QLabel* hintLabel = new QLabel(
        "Управление клиентами, торговыми точками, договорами аренды, платежами и правами доступа.",
        this
    );
    hintLabel->setObjectName("Subtitle");
    hintLabel->setWordWrap(true);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(hintLabel);

    QFrame* infoCard = new QFrame(this);
    infoCard->setObjectName("InfoCard");
    infoCard->setMinimumWidth(310);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(18, 14, 18, 14);

    QLabel* infoTitle = new QLabel("Роль: администратор", this);
    infoTitle->setObjectName("CardTitle");

    QLabel* infoText = new QLabel(
        "Доступны операции CRUD, оформление договоров и внесение ежемесячных платежей.",
        this
    );
    infoText->setObjectName("CardText");
    infoText->setWordWrap(true);

    infoLayout->addWidget(infoTitle);
    infoLayout->addWidget(infoText);

    topLayout->addLayout(titleLayout, 1);
    topLayout->addWidget(infoCard);

    rootLayout->addWidget(topBar);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget* content = new QWidget(scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);

    auto makeButton = [this](const QString& number, const QString& title, const QString& description) {
        QPushButton* button = new QPushButton(number + "  " + title + "\n" + description, this);
        button->setMinimumHeight(72);
        button->setToolTip(description);
        return button;
    };

    auto addGroup = [this, contentLayout](const QString& title) {
        QGroupBox* group = new QGroupBox(title, this);
        QGridLayout* grid = new QGridLayout(group);
        grid->setContentsMargins(16, 18, 16, 16);
        grid->setHorizontalSpacing(14);
        grid->setVerticalSpacing(14);
        contentLayout->addWidget(group);
        return grid;
    };

    auto addTableButton = [this, makeButton](QGridLayout* grid,
                                             int row,
                                             int col,
                                             const QString& number,
                                             const QString& buttonText,
                                             const QString& key,
                                             const QString& title,
                                             const QString& description) {
        QPushButton* button = makeButton(number, buttonText, description);
        grid->addWidget(button, row, col);
        connect(button, &QPushButton::clicked, this, [this, key, title]() {
            openTable(key, title);
        });
    };

    QGridLayout* mainGrid = addGroup("Разделы предметной области");
    addTableButton(mainGrid, 0, 0, "01", "Клиенты", "clients", "Клиенты", "Организации-арендаторы и контактные данные");
    addTableButton(mainGrid, 0, 1, "02", "Торговые точки", "retail_spaces", "Торговые точки", "Этаж, площадь, кондиционер и ставка за день");
    addTableButton(mainGrid, 1, 0, "03", "Договоры аренды", "rental_contracts", "Договоры аренды", "Клиент и дата заключения договора");
    addTableButton(mainGrid, 1, 1, "04", "Периоды аренды", "rented_spaces", "Арендуемые точки", "Связь договора, точки и срока аренды");
    addTableButton(mainGrid, 2, 0, "05", "Платежи", "payments", "Платежи", "Учет поступивших платежей по договорам");

    QGridLayout* usersGrid = addGroup("Учетные записи и доступ");
    addTableButton(usersGrid, 0, 0, "06", "Пользователи", "users", "Пользователи", "Логины и хэши паролей учетных записей");
    addTableButton(usersGrid, 0, 1, "07", "Роли", "roles", "Роли", "Справочник ролей системы");
    addTableButton(usersGrid, 1, 0, "08", "Назначение ролей", "users_roles", "Пользователи — роли", "Связь пользователей с правами доступа");

    QGroupBox* actionsGroup = new QGroupBox("Операции администратора", this);
    QHBoxLayout* actionsLayout = new QHBoxLayout(actionsGroup);
    actionsLayout->setContentsMargins(16, 18, 16, 16);
    actionsLayout->setSpacing(14);

    QPushButton* createContractButton = new QPushButton("+  Оформить договор аренды\nСоздать договор и указать период аренды", this);
    QPushButton* addPaymentButton = new QPushButton("₽  Зарегистрировать платеж\nВнести оплату по договору и торговой точке", this);

    createContractButton->setObjectName("PrimaryButton");
    addPaymentButton->setObjectName("GreenButton");
    createContractButton->setMinimumHeight(76);
    addPaymentButton->setMinimumHeight(76);

    actionsLayout->addWidget(createContractButton);
    actionsLayout->addWidget(addPaymentButton);

    contentLayout->addWidget(actionsGroup);

    connect(createContractButton, &QPushButton::clicked, this, &AdminWindow::showCreateContractDialog);
    connect(addPaymentButton, &QPushButton::clicked, this, &AdminWindow::showAddPaymentDialog);

    contentLayout->addStretch();
    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea);

    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer, this, [this](const QString& message) {
        if (message == "CONTRACT_CREATED")
            QMessageBox::information(this, "Готово", "Договор аренды успешно создан.");
        else if (message == "PAYMENT_ADDED")
            QMessageBox::information(this, "Готово", "Платеж успешно добавлен.");
        else if (message.startsWith("ERROR&") || message.startsWith("ACCESS_DENIED&"))
            QMessageBox::warning(this, "Ошибка", message.section('&', 1));
    });

    setWindowTitle("Административный центр");
    resize(1120, 760);
}

void AdminWindow::openTable(const QString& tableKey, const QString& title)
{
    AdminTableWindow* window = new AdminTableWindow(tableKey, title);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void AdminWindow::showCreateContractDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Оформление договора аренды");
    dialog.setStyleSheet(
        "QDialog { background-color: #f7f9fc; font-family: Segoe UI; font-size: 14px; }"
        "QLineEdit, QSpinBox, QDateEdit { background: white; border: 1px solid #cfd7e6; border-radius: 8px; padding: 7px; }"
        "QPushButton { padding: 7px 14px; }"
    );

    QFormLayout* form = new QFormLayout(&dialog);
    form->setContentsMargins(18, 18, 18, 18);
    form->setSpacing(12);

    QSpinBox* clientIdEdit = new QSpinBox(&dialog);
    QSpinBox* spaceIdEdit = new QSpinBox(&dialog);
    QDateEdit* startDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    QDateEdit* endDateEdit = new QDateEdit(QDate::currentDate().addMonths(1), &dialog);

    clientIdEdit->setRange(1, 1000000);
    spaceIdEdit->setRange(1, 1000000);
    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    form->addRow("ID клиента:", clientIdEdit);
    form->addRow("ID торговой точки:", spaceIdEdit);
    form->addRow("Дата начала:", startDateEdit);
    form->addRow("Дата окончания:", endDateEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        AdminController::instance()->createContract(clientIdEdit->value(),
                                                    spaceIdEdit->value(),
                                                    startDateEdit->date().toString("yyyy-MM-dd"),
                                                    endDateEdit->date().toString("yyyy-MM-dd"));
    }
}

void AdminWindow::showAddPaymentDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Добавление платежа");
    dialog.setStyleSheet(
        "QDialog { background-color: #f7f9fc; font-family: Segoe UI; font-size: 14px; }"
        "QLineEdit, QSpinBox, QDateEdit { background: white; border: 1px solid #cfd7e6; border-radius: 8px; padding: 7px; }"
        "QPushButton { padding: 7px 14px; }"
    );

    QFormLayout* form = new QFormLayout(&dialog);
    form->setContentsMargins(18, 18, 18, 18);
    form->setSpacing(12);

    QSpinBox* contractIdEdit = new QSpinBox(&dialog);
    QSpinBox* spaceIdEdit = new QSpinBox(&dialog);
    QDateEdit* paymentDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    QLineEdit* amountEdit = new QLineEdit(&dialog);

    contractIdEdit->setRange(1, 1000000);
    spaceIdEdit->setRange(1, 1000000);
    paymentDateEdit->setCalendarPopup(true);
    paymentDateEdit->setDisplayFormat("yyyy-MM-dd");
    amountEdit->setPlaceholderText("Например 45000.00");

    form->addRow("ID договора:", contractIdEdit);
    form->addRow("ID торговой точки:", spaceIdEdit);
    form->addRow("Дата оплаты:", paymentDateEdit);
    form->addRow("Сумма платежа:", amountEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        AdminController::instance()->addPayment(contractIdEdit->value(),
                                                spaceIdEdit->value(),
                                                paymentDateEdit->date().toString("yyyy-MM-dd"),
                                                amountEdit->text().trimmed());
    }
}
