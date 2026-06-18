#include "authwindow.h"
#include "adminwindow.h"
#include "clientwindow.h"
#include "../Controllers/authcontroller.h"
#include "../Network/singletonclient.h"
#include "../Styles/thememanager.h"

#include <QApplication>
#include <QFormLayout>
#include <QHBoxLayout>
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
    QLabel* titleLabel = new QLabel(QString::fromUtf8("Система аренды торговых площадей"), this);
    titleLabel->setObjectName("MainTitle");

    QLabel* subLabel = new QLabel(QString::fromUtf8("Авторизация пользователя и выбор темы интерфейса."), this);
    subLabel->setObjectName("SoftText");
    subLabel->setWordWrap(true);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subLabel);

    themeButton = new QPushButton(this);
    themeButton->setMinimumHeight(40);
    connect(themeButton, &QPushButton::clicked, this, &AuthWindow::onThemeSwitchClicked);
    updateThemeButton();

    topLayout->addLayout(titleLayout, 1);
    topLayout->addWidget(themeButton, 0, Qt::AlignTop);

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

    loginButton = new QPushButton(QString::fromUtf8("Войти"), this);
    statusLabel = new QLabel(QString::fromUtf8("Запустите сервер и выполните вход"), this);
    statusLabel->setObjectName("HintLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(loginButton);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();

    connect(loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);

    setWindowTitle(QString::fromUtf8("Авторизация"));
    resize(560, 300);
}

void AuthWindow::updateThemeButton()
{
    if (themeButton)
        themeButton->setText(ThemeManager::switchButtonText());
}

void AuthWindow::onThemeSwitchClicked()
{
    ThemeManager::toggleTheme(qApp);
    updateThemeButton();
}

void AuthWindow::onLoginClicked()
{
    statusLabel->setText(QString::fromUtf8("Отправка запроса авторизации..."));
    AuthController::instance()->login(loginEdit->text().trimmed(), passwordEdit->text());
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
