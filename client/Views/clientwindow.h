#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDateEdit>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>

class ThemeToggleSwitch;

#include "../Models/user.h"
#include "../Models/client.h"
#include "../Models/contractinfo.h"
#include "../Models/paymentinfo.h"
#include "../Models/retailspace.h"

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
    QList<RetailSpace> parseFreeSpaces(const QString& message) const;
    QList<ContractInfo> parseContracts(const QString& message) const;
    QList<PaymentInfo> parsePayments(const QString& message) const;
    Client parseClientProfile(const QString& message) const;

    void showFreeSpaces(const QList<RetailSpace>& spaces);
    void showContracts(const QList<ContractInfo>& contracts);
    void showPayments(const QList<PaymentInfo>& payments);
    void showClientProfile(const Client& client);

    void setupTable(QTableWidget* table, const QStringList& headers, int rowCount);
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
