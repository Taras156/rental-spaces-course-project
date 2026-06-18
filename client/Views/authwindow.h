#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

#include "../Models/user.h"

class AuthWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();
    void onRegistrationClicked();
    void onLoginSuccess(const User& user);
    void onLoginFailed(const QString& errorText);
    void onRegistrationSuccess();
    void onRegistrationFailed(const QString& errorText);
    void onThemeSwitchClicked();

private:
    void setupUi();
    void updateThemeButton();

private:
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* registrationButton;
    QPushButton* themeButton;
    QLabel* statusLabel;
};

#endif // AUTHWINDOW_H
