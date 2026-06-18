#include "authwindow.h"
#include "adminwindow.h"
#include "clientwindow.h"
#include "../Controllers/authcontroller.h"
#include "../Network/singletonclient.h"

#include <QFormLayout>
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
            this, [this]() { statusLabel->setText("Подключено к серверу. Введите логин и пароль."); });
    connect(SingletonClient::getInstance(), &SingletonClient::errorOccurred,
            this, [this](const QString& err) { statusLabel->setText("Ошибка подключения: " + err); });
}

void AuthWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QLabel* titleLabel = new QLabel("Система аренды торговых площадей", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);

    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    loginEdit->setPlaceholderText("Логин");
    passwordEdit->setPlaceholderText("Пароль");

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow("Логин:", loginEdit);
    formLayout->addRow("Пароль:", passwordEdit);

    loginButton = new QPushButton("Войти", this);
    statusLabel = new QLabel("Запустите сервер и выполните вход", this);
    statusLabel->setWordWrap(true);
    statusLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(loginButton);
    mainLayout->addWidget(statusLabel);

    connect(loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);

    setWindowTitle("Авторизация");
    resize(520, 260);
}

void AuthWindow::onLoginClicked()
{
    statusLabel->setText("Отправка запроса авторизации...");
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
    statusLabel->setText("Ошибка входа: " + errorText);
}
