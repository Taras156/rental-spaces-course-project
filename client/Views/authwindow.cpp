#include "authwindow.h"
#include "adminwindow.h"
#include "clientwindow.h"
#include "../Controllers/authcontroller.h"
#include "../Network/singletonclient.h"
#include "../Styles/thememanager.h"
#include "../Styles/themetoggleswitch.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

AuthWindow::AuthWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();

    connect(AuthController::instance(), &AuthController::loginSuccess,
            this, &AuthWindow::onLoginSuccess);
    connect(AuthController::instance(), &AuthController::loginFailed,
            this, &AuthWindow::onLoginFailed);
    connect(AuthController::instance(), &AuthController::registrationSuccess,
            this, &AuthWindow::onRegistrationSuccess);
    connect(AuthController::instance(), &AuthController::registrationFailed,
            this, &AuthWindow::onRegistrationFailed);

    connect(SingletonClient::getInstance(), &SingletonClient::connected,
            this, [this]() { statusLabel->setText(QString::fromUtf8("Подключено к серверу. Введите логин и пароль.")); });
    connect(SingletonClient::getInstance(), &SingletonClient::errorOccurred,
            this, [this](const QString& err) { statusLabel->setText(QString::fromUtf8("Ошибка подключения: ") + err); });
}

void AuthWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(14);

    QHBoxLayout* topLayout = new QHBoxLayout();

    QVBoxLayout* titleLayout = new QVBoxLayout();
    QLabel* titleLabel = new QLabel(QString::fromUtf8("Авторизация"), this);
    titleLabel->setObjectName("MainTitle");

    titleLayout->addWidget(titleLabel);

    themeSwitch = new ThemeToggleSwitch(this);
    connect(themeSwitch, &ThemeToggleSwitch::clicked, this, &AuthWindow::onThemeSwitchClicked);
    updateThemeSwitch();

    topLayout->addLayout(titleLayout, 1);
    topLayout->addWidget(themeSwitch, 0, Qt::AlignTop);

    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    loginEdit->setPlaceholderText(QString::fromUtf8("Логин"));
    passwordEdit->setPlaceholderText(QString::fromUtf8("Пароль"));

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(16);
    formLayout->setVerticalSpacing(12);
    formLayout->addRow(QString::fromUtf8("Логин:"), loginEdit);
    formLayout->addRow(QString::fromUtf8("Пароль:"), passwordEdit);

    QHBoxLayout* authButtonsLayout = new QHBoxLayout();
    loginButton = new QPushButton(QString::fromUtf8("Войти"), this);
    registrationButton = new QPushButton(QString::fromUtf8("Регистрация"), this);
    loginButton->setMinimumHeight(42);
    registrationButton->setMinimumHeight(42);

    authButtonsLayout->addWidget(loginButton);
    authButtonsLayout->addWidget(registrationButton);

    statusLabel = new QLabel(QString::fromUtf8("Запустите сервер и выполните вход"), this);
    statusLabel->setObjectName("HintLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(authButtonsLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();

    connect(loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);
    connect(registrationButton, &QPushButton::clicked, this, &AuthWindow::onRegistrationClicked);

    setWindowTitle(QString::fromUtf8("Авторизация"));
    resize(620, 300);
}

void AuthWindow::updateThemeSwitch()
{
    if (themeSwitch)
        themeSwitch->setChecked(ThemeManager::currentTheme() == ThemeManager::Theme9_SkyLight);
}

void AuthWindow::onThemeSwitchClicked()
{
    ThemeManager::toggleTheme(qApp);
    updateThemeSwitch();
}

void AuthWindow::onLoginClicked()
{
    statusLabel->setText(QString::fromUtf8("Отправка запроса авторизации..."));
    AuthController::instance()->login(loginEdit->text().trimmed(), passwordEdit->text());
}

void AuthWindow::onRegistrationClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Регистрация арендатора"));
    dialog.resize(560, 460);

    QVBoxLayout* rootLayout = new QVBoxLayout(&dialog);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("Регистрация нового клиента"), &dialog);
    titleLabel->setObjectName("SectionTitle");

    rootLayout->addWidget(titleLabel);

    QFormLayout* form = new QFormLayout();
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(12);

    QLineEdit* regLoginEdit = new QLineEdit(&dialog);
    QLineEdit* regPasswordEdit = new QLineEdit(&dialog);
    QLineEdit* regPasswordRepeatEdit = new QLineEdit(&dialog);
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* addressEdit = new QLineEdit(&dialog);
    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    QLineEdit* requisitesEdit = new QLineEdit(&dialog);
    QLineEdit* contactPersonEdit = new QLineEdit(&dialog);

    regPasswordEdit->setEchoMode(QLineEdit::Password);
    regPasswordRepeatEdit->setEchoMode(QLineEdit::Password);

    regLoginEdit->setPlaceholderText(QString::fromUtf8("Например, client_gamma"));
    regPasswordEdit->setPlaceholderText(QString::fromUtf8("Не менее 8 символов, A, 1, !"));
    regPasswordRepeatEdit->setPlaceholderText(QString::fromUtf8("Повторите пароль"));
    nameEdit->setPlaceholderText(QString::fromUtf8("Название организации"));
    addressEdit->setPlaceholderText(QString::fromUtf8("Адрес"));
    phoneEdit->setPlaceholderText(QString::fromUtf8("+7..."));
    requisitesEdit->setPlaceholderText(QString::fromUtf8("ИНН / КПП / расчетный счет"));
    contactPersonEdit->setPlaceholderText(QString::fromUtf8("ФИО контактного лица"));

    form->addRow(QString::fromUtf8("Логин:"), regLoginEdit);
    form->addRow(QString::fromUtf8("Пароль:"), regPasswordEdit);
    form->addRow(QString::fromUtf8("Повтор пароля:"), regPasswordRepeatEdit);
    form->addRow(QString::fromUtf8("Организация:"), nameEdit);
    form->addRow(QString::fromUtf8("Адрес:"), addressEdit);
    form->addRow(QString::fromUtf8("Телефон:"), phoneEdit);
    form->addRow(QString::fromUtf8("Реквизиты:"), requisitesEdit);
    form->addRow(QString::fromUtf8("Контактное лицо:"), contactPersonEdit);

    rootLayout->addLayout(form);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    box->button(QDialogButtonBox::Ok)->setText(QString::fromUtf8("Зарегистрироваться"));
    box->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("Отмена"));

    connect(box, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (regLoginEdit->text().trimmed().isEmpty() ||
            regPasswordEdit->text().isEmpty() ||
            nameEdit->text().trimmed().isEmpty() ||
            addressEdit->text().trimmed().isEmpty() ||
            phoneEdit->text().trimmed().isEmpty() ||
            requisitesEdit->text().trimmed().isEmpty() ||
            contactPersonEdit->text().trimmed().isEmpty())
        {
            QMessageBox::warning(&dialog, QString::fromUtf8("Регистрация"),
                                 QString::fromUtf8("Заполните все поля."));
            return;
        }

        if (regPasswordEdit->text() != regPasswordRepeatEdit->text())
        {
            QMessageBox::warning(&dialog, QString::fromUtf8("Регистрация"),
                                 QString::fromUtf8("Пароли не совпадают."));
            return;
        }

        AuthController::instance()->registerClient(
            regLoginEdit->text().trimmed(),
            regPasswordEdit->text(),
            nameEdit->text().trimmed(),
            addressEdit->text().trimmed(),
            phoneEdit->text().trimmed(),
            requisitesEdit->text().trimmed(),
            contactPersonEdit->text().trimmed()
        );

        dialog.accept();
        statusLabel->setText(QString::fromUtf8("Отправлен запрос регистрации..."));
    });

    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    rootLayout->addWidget(box);

    dialog.exec();
}

void AuthWindow::onLoginSuccess(const User& user)
{
    QWidget* nextWindow = nullptr;
    if (user.isAdmin())
        nextWindow = new AdminWindow(user);
    else
        nextWindow = new ClientWindow(user);

    nextWindow->setAttribute(Qt::WA_DeleteOnClose);
    nextWindow->show();
    close();
}

void AuthWindow::onLoginFailed(const QString& errorText)
{
    statusLabel->setText(QString::fromUtf8("Ошибка входа: ") + errorText);
}

void AuthWindow::onRegistrationSuccess()
{
    statusLabel->setText(QString::fromUtf8("Регистрация выполнена. Теперь можно войти под новым логином."));
    QMessageBox::information(this, QString::fromUtf8("Регистрация"),
                             QString::fromUtf8("Клиент успешно зарегистрирован. Теперь можно выполнить вход."));
}

void AuthWindow::onRegistrationFailed(const QString& errorText)
{
    statusLabel->setText(QString::fromUtf8("Ошибка регистрации: ") + errorText);
    QMessageBox::warning(this, QString::fromUtf8("Ошибка регистрации"), errorText);
}
