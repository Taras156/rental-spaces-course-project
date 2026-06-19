#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDateEdit>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>

class ThemeToggleSwitch;

#include "../Models/user.h"

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void refreshAll();
    void loadFreeSpaces();
    void saveProfile();
    void changePassword();
    void handleServerMessage(const QString& message);
    void toggleTheme();

private:
    void setupUi();
    QString decodeValue(const QString& value) const;
    void fillTable(QTableWidget* table, const QStringList& headers, const QList<QStringList>& rows);
    void updateThemeSwitch();

private:
    User m_user;

    QTabWidget* tabs;
    QTableWidget* freeSpacesTable;
    QTableWidget* contractsTable;
    QTableWidget* paymentsTable;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    ThemeToggleSwitch* themeSwitch;

    QLineEdit* nameEdit;
    QLineEdit* addressEdit;
    QLineEdit* phoneEdit;
    QLineEdit* requisitesEdit;
    QLineEdit* contactPersonEdit;
    QLineEdit* newPasswordEdit;
};

#endif // CLIENTWINDOW_H
