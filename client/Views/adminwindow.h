#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "../Models/user.h"

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void openTable(const QString& tableKey, const QString& title);
    void showCreateContractDialog();
    void showAddPaymentDialog();
    void toggleTheme();

private:
    User m_user;
};

#endif // ADMINWINDOW_H
