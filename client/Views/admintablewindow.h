#ifndef ADMINTABLEWINDOW_H
#define ADMINTABLEWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>

class AdminTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminTableWindow(const QString& tableKey,
                              const QString& windowTitle,
                              QWidget* parent = nullptr);

private slots:
    void loadData();
    void addRecord();
    void editRecord();
    void deleteRecord();
    void handleServerMessage(const QString& message);

private:
    void setupUi();
    QString decodeValue(const QString& value) const;
    QStringList showEditDialog(const QString& title, const QStringList& columns, bool fillCurrentValues);
    QString valueFromSelectedRow(const QString& columnName) const;
    QString displayName(const QString& columnName) const;
    QStringList displayNames(const QStringList& columnNames) const;

private:
    QString m_tableKey;
    QString m_title;
    int m_pkCount;
    QStringList m_columns;
    QStringList m_addColumns;
    QStringList m_updateColumns;

    QTableWidget* table;
    QPushButton* refreshButton;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
};

#endif // ADMINTABLEWINDOW_H
