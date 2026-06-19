#ifndef FINANCEREPORTWINDOW_H
#define FINANCEREPORTWINDOW_H

#include <QDateEdit>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>

class FinanceReportWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FinanceReportWindow(QWidget* parent = nullptr);

private slots:
    void loadReport();
    void handleServerMessage(const QString& message);

private:
    void setupUi();
    QString decodeValue(const QString& value) const;
    QString formatMoney(double value) const;
    QString formatPercent(double value) const;
    QLabel* createCard(const QString& title, QLabel** valueLabel);

private:
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QPushButton* loadButton;

    QLabel* plannedValueLabel;
    QLabel* paidValueLabel;
    QLabel* debtValueLabel;
    QLabel* percentValueLabel;
    QLabel* activeContractsValueLabel;
    QLabel* rentedAreaValueLabel;
    QLabel* statusLabel;

    QTableWidget* reportTable;
};

#endif // FINANCEREPORTWINDOW_H
