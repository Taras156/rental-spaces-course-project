#include "adminwindow.h"
#include "admintablewindow.h"
#include "financereportwindow.h"
#include "../Controllers/admincontroller.h"
#include "../Network/singletonclient.h"
#include "../Styles/thememanager.h"
#include "../Styles/themetoggleswitch.h"

#include <QApplication>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPlainTextEdit>
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

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QVBoxLayout* titleBlock = new QVBoxLayout();

    QLabel* titleLabel = new QLabel(QString::fromUtf8("Административный центр"), this);
    titleLabel->setObjectName("MainTitle");
    titleBlock->addWidget(titleLabel);

    ThemeToggleSwitch* themeSwitch = new ThemeToggleSwitch(this);
    connect(themeSwitch, &ThemeToggleSwitch::clicked, this, &AdminWindow::toggleTheme);

    headerLayout->addLayout(titleBlock, 1);
    headerLayout->addWidget(themeSwitch, 0, Qt::AlignTop | Qt::AlignRight);

    mainLayout->addLayout(headerLayout);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget* content = new QWidget(scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(16);

    auto makeButton = [this](const QString& text, const QString& tooltip) {
        QPushButton* button = new QPushButton(text, this);
        button->setMinimumHeight(52);
        button->setToolTip(tooltip);
        button->setStyleSheet("QPushButton { text-align: left; padding-left: 14px; }");
        return button;
    };

    auto addGroup = [this, contentLayout](const QString& title) {
        QGroupBox* group = new QGroupBox(title, this);
        QGridLayout* grid = new QGridLayout(group);
        grid->setHorizontalSpacing(14);
        grid->setVerticalSpacing(12);
        contentLayout->addWidget(group);
        return grid;
    };

    auto addTableButton = [this, makeButton](QGridLayout* grid, int row, int col,
                                             const QString& buttonText, const QString& key,
                                             const QString& title, const QString& tooltip) {
        QPushButton* button = makeButton(buttonText, tooltip);
        grid->addWidget(button, row, col);
        connect(button, &QPushButton::clicked, this, [this, key, title]() { openTable(key, title); });
    };

    QGridLayout* mainGrid = addGroup(QString::fromUtf8("Основные таблицы"));
    addTableButton(mainGrid, 0, 0, QString::fromUtf8("Клиенты"), "clients", QString::fromUtf8("Клиенты"), QString::fromUtf8("Потенциальные и текущие арендаторы"));
    addTableButton(mainGrid, 0, 1, QString::fromUtf8("Торговые точки"), "retail_spaces", QString::fromUtf8("Торговые точки"), QString::fromUtf8("Этаж, площадь, кондиционер и стоимость аренды"));
    addTableButton(mainGrid, 1, 0, QString::fromUtf8("Договоры аренды"), "rental_contracts", QString::fromUtf8("Договоры аренды"), QString::fromUtf8("Клиент и дата заключения договора"));
    addTableButton(mainGrid, 1, 1, QString::fromUtf8("Арендуемые точки"), "rented_spaces", QString::fromUtf8("Арендуемые точки"), QString::fromUtf8("Связь договора с торговой точкой и периодом аренды"));
    addTableButton(mainGrid, 2, 0, QString::fromUtf8("Платежи"), "payments", QString::fromUtf8("Платежи"), QString::fromUtf8("Ежемесячные платежи арендаторов"));

    QGridLayout* usersGrid = addGroup(QString::fromUtf8("Пользователи и права доступа"));
    addTableButton(usersGrid, 0, 0, QString::fromUtf8("Пользователи"), "users", QString::fromUtf8("Пользователи"), QString::fromUtf8("Логины и хэши паролей"));
    addTableButton(usersGrid, 0, 1, QString::fromUtf8("Роли"), "roles", QString::fromUtf8("Роли"), QString::fromUtf8("Справочник ролей"));
    addTableButton(usersGrid, 1, 0, QString::fromUtf8("Пользователи — роли"), "users_roles", QString::fromUtf8("Пользователи — роли"), QString::fromUtf8("Назначение ролей пользователям"));

    QGridLayout* actionGrid = addGroup(QString::fromUtf8("Быстрые действия"));
    QPushButton* createContractButton = makeButton(QString::fromUtf8("Оформить договор аренды"), QString::fromUtf8("Создать один договор и привязать к нему несколько торговых точек"));
    QPushButton* addPaymentButton = makeButton(QString::fromUtf8("Добавить платеж"), QString::fromUtf8("Внести ежемесячный платеж по договору и торговой точке"));
    QPushButton* financeReportButton = makeButton(QString::fromUtf8("Финансовый отчет"), QString::fromUtf8("Показать начисления, оплаты и задолженность по месяцам"));
    actionGrid->addWidget(createContractButton, 0, 0);
    actionGrid->addWidget(addPaymentButton, 0, 1);
    actionGrid->addWidget(financeReportButton, 1, 0, 1, 2);

    connect(createContractButton, &QPushButton::clicked, this, &AdminWindow::showCreateContractDialog);
    connect(addPaymentButton, &QPushButton::clicked, this, &AdminWindow::showAddPaymentDialog);
    connect(financeReportButton, &QPushButton::clicked, this, &AdminWindow::showFinanceReport);

    contentLayout->addStretch();
    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);

    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer, this, [this](const QString& message) {
        if (message == "CONTRACT_CREATED")
            QMessageBox::information(this, QString::fromUtf8("Готово"), QString::fromUtf8("Договор аренды создан. К нему привязаны выбранные торговые точки."));
        else if (message == "PAYMENT_ADDED")
            QMessageBox::information(this, QString::fromUtf8("Готово"), QString::fromUtf8("Платеж добавлен."));
        else if (message.startsWith("ERROR&") || message.startsWith("ACCESS_DENIED&"))
            QMessageBox::warning(this, QString::fromUtf8("Ошибка"), message.section('&', 1));
    });

    setWindowTitle(QString::fromUtf8("Администратор"));
    resize(1160, 760);
}

void AdminWindow::toggleTheme()
{
    ThemeManager::toggleTheme(qApp);

    const auto switches = findChildren<ThemeToggleSwitch*>();
    for (ThemeToggleSwitch* themeSwitch : switches)
        themeSwitch->setChecked(ThemeManager::currentTheme() == ThemeManager::Theme9_SkyLight);
}

void AdminWindow::openTable(const QString& tableKey, const QString& title)
{
    AdminTableWindow* window = new AdminTableWindow(tableKey, title);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}


void AdminWindow::showFinanceReport()
{
    FinanceReportWindow* window = new FinanceReportWindow(this);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void AdminWindow::showCreateContractDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Оформление договора аренды"));
    dialog.resize(620, 440);

    QVBoxLayout* rootLayout = new QVBoxLayout(&dialog);
    QLabel* title = new QLabel(QString::fromUtf8("Создание договора аренды"), &dialog);
    title->setObjectName("SectionTitle");

    QLabel* note = new QLabel(
        QString::fromUtf8("Укажите ID клиента, период аренды и несколько ID торговых точек. "
                          "Будет создан один договор, а каждая выбранная точка попадет в таблицу «Арендуемые торговые точки»."),
        &dialog
    );
    note->setObjectName("HintLabel");
    note->setWordWrap(true);

    rootLayout->addWidget(title);
    rootLayout->addWidget(note);

    QFormLayout* form = new QFormLayout();
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    QSpinBox* clientIdEdit = new QSpinBox(&dialog);
    QDateEdit* startDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    QDateEdit* endDateEdit = new QDateEdit(QDate::currentDate().addMonths(1), &dialog);
    QLineEdit* spaceIdsEdit = new QLineEdit(&dialog);
    QPlainTextEdit* helpText = new QPlainTextEdit(&dialog);

    clientIdEdit->setRange(1, 1000000);
    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    startDateEdit->setMinimumWidth(150);
    endDateEdit->setMinimumWidth(150);

    spaceIdsEdit->setPlaceholderText(QString::fromUtf8("Например: 231, 232, 233"));

    helpText->setReadOnly(true);
    helpText->setMaximumHeight(95);
    helpText->setPlainText(QString::fromUtf8(
        "Как заполнять:\n"
        "1) сначала выберите клиента;\n"
        "2) укажите период аренды;\n"
        "3) введите ID свободных торговых точек через запятую, пробел или точку с запятой.\n"
        "Пример: 231, 232, 233"
    ));

    form->addRow(QString::fromUtf8("ID клиента:"), clientIdEdit);
    form->addRow(QString::fromUtf8("Дата начала:"), startDateEdit);
    form->addRow(QString::fromUtf8("Дата окончания:"), endDateEdit);
    form->addRow(QString::fromUtf8("ID торговых точек:"), spaceIdsEdit);

    rootLayout->addLayout(form);
    rootLayout->addWidget(helpText);

    QLabel* warning = new QLabel(
        QString::fromUtf8("Важно: если хотя бы одна точка занята в выбранный период, договор не будет создан."),
        &dialog
    );
    warning->setObjectName("HintLabel");
    warning->setWordWrap(true);
    rootLayout->addWidget(warning);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    box->button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("Создать договор"));
    box->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("Отмена"));
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    rootLayout->addWidget(box);

    if (dialog.exec() == QDialog::Accepted)
    {
        if (startDateEdit->date() > endDateEdit->date())
        {
            QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                                 QString::fromUtf8("Дата начала не может быть позже даты окончания."));
            return;
        }

        QString rawText = spaceIdsEdit->text().trimmed();
        if (rawText.isEmpty())
        {
            QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                                 QString::fromUtf8("Введите хотя бы одну торговую точку."));
            return;
        }

        QStringList spaceIds = rawText.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
        QStringList normalizedIds;
        QSet<QString> uniqueIds;

        for (const QString& value : spaceIds)
        {
            bool ok = false;
            int id = value.toInt(&ok);

            if (!ok || id <= 0)
            {
                QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                                     QString::fromUtf8("Некорректный ID торговой точки: ") + value);
                return;
            }

            QString idText = QString::number(id);
            if (!uniqueIds.contains(idText))
            {
                uniqueIds.insert(idText);
                normalizedIds << idText;
            }
        }

        AdminController::instance()->createContract(clientIdEdit->value(),
                                                    normalizedIds,
                                                    startDateEdit->date().toString("yyyy-MM-dd"),
                                                    endDateEdit->date().toString("yyyy-MM-dd"));
    }
}

void AdminWindow::showAddPaymentDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Добавление платежа"));
    dialog.resize(460, 260);

    QVBoxLayout* rootLayout = new QVBoxLayout(&dialog);
    QLabel* title = new QLabel(QString::fromUtf8("Добавление платежа"), &dialog);
    title->setObjectName("SectionTitle");
    QLabel* note = new QLabel(QString::fromUtf8("Укажите договор, торговую точку, дату и сумму оплаты."), &dialog);
    note->setObjectName("HintLabel");
    note->setWordWrap(true);

    rootLayout->addWidget(title);
    rootLayout->addWidget(note);

    QFormLayout* form = new QFormLayout();
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    QSpinBox* contractIdEdit = new QSpinBox(&dialog);
    QSpinBox* spaceIdEdit = new QSpinBox(&dialog);
    QDateEdit* paymentDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    QLineEdit* amountEdit = new QLineEdit(&dialog);

    contractIdEdit->setRange(1, 1000000);
    spaceIdEdit->setRange(1, 1000000);
    paymentDateEdit->setCalendarPopup(true);
    paymentDateEdit->setDisplayFormat("yyyy-MM-dd");
    paymentDateEdit->setMinimumWidth(150);
    amountEdit->setPlaceholderText(QString::fromUtf8("Например, 25000"));

    form->addRow(QString::fromUtf8("ID договора:"), contractIdEdit);
    form->addRow(QString::fromUtf8("ID торговой точки:"), spaceIdEdit);
    form->addRow(QString::fromUtf8("Дата оплаты:"), paymentDateEdit);
    form->addRow(QString::fromUtf8("Сумма платежа:"), amountEdit);

    rootLayout->addLayout(form);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    box->button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("Добавить"));
    box->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("Отмена"));
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    rootLayout->addWidget(box);

    if (dialog.exec() == QDialog::Accepted)
    {
        AdminController::instance()->addPayment(contractIdEdit->value(), spaceIdEdit->value(),
                                                paymentDateEdit->date().toString("yyyy-MM-dd"),
                                                amountEdit->text().trimmed());
    }
}
