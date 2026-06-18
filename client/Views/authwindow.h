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
    void onLoginSuccess(const User& user);
    void onLoginFailed(const QString& errorText);

private:
    void setupUi();

private:
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QLabel* statusLabel;
};

#endif // AUTHWINDOW_H
