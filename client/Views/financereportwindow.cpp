#include "financereportwindow.h"
#include "../Controllers/admincontroller.h"
#include "../Network/singletonclient.h"

#include <QBrush>
#include <QColor>
#include <QDate>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLocale>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

FinanceReportWindow::FinanceReportWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();

    connect(loadButton, &QPushButton::clicked, this, &FinanceReportWindow::loadReport);
    connect(SingletonClient::getInstance(), &SingletonClient::messageFromServer,
            this, &FinanceReportWindow::handleServerMessage);

    loadReport();
}

void FinanceReportWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("Финансовый отчет по аренде"), this);
    titleLabel->setObjectName("MainTitle");

    QLabel* hintLabel = new QLabel(
        QString::fromUtf8("Отчет показывает ежемесячные начисления, поступившие платежи и задолженность арендаторов."),
        this);
    hintLabel->setObjectName("HintLabel");
    hintLabel->setWordWrap(true);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(hintLabel);

    QHBoxLayout* filterLayout = new QHBoxLayout();

    startDateEdit = new QDateEdit(QDate::currentDate().addDays(1 - QDate::currentDate().day()), this);
    endDateEdit = new QDateEdit(QDate::currentDate().addDays(1 - QDate::currentDate().day()).addMonths(1).addDays(-1), this);
    loadButton = new QPushButton(QString::fromUtf8("Сформировать отчет"), this);

    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);
    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");
    loadButton->setMinimumHeight(38);

    filterLayout->addWidget(new QLabel(QString::fromUtf8("Период с:"), this));
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(new QLabel(QString::fromUtf8("по:"), this));
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(loadButton);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);

    QGridLayout* cardsLayout = new QGridLayout();
    cardsLayout->setHorizontalSpacing(12);
    cardsLayout->setVerticalSpacing(12);

    cardsLayout->addWidget(createCard(QString::fromUtf8("Начислено"), &plannedValueLabel), 0, 0);
    cardsLayout->addWidget(createCard(QString::fromUtf8("Поступило"), &paidValueLabel), 0, 1);
    cardsLayout->addWidget(createCard(QString::fromUtf8("Задолженность"), &debtValueLabel), 0, 2);
    cardsLayout->addWidget(createCard(QString::fromUtf8("Оплата"), &percentValueLabel), 0, 3);
    cardsLayout->addWidget(createCard(QString::fromUtf8("Активных договоров"), &activeContractsValueLabel), 1, 0);
    cardsLayout->addWidget(createCard(QString::fromUtf8("Сданная площадь"), &rentedAreaValueLabel), 1, 1);

    mainLayout->addLayout(cardsLayout);

    reportTable = new QTableWidget(this);
    reportTable->setAlternatingRowColors(true);
    reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reportTable->horizontalHeader()->setStretchLastSection(true);
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    mainLayout->addWidget(reportTable, 1);

    statusLabel = new QLabel(QString::fromUtf8("Готово к формированию отчета."), this);
    statusLabel->setObjectName("HintLabel");
    mainLayout->addWidget(statusLabel);

    setWindowTitle(QString::fromUtf8("Финансовый отчет"));
    resize(1200, 720);
}

QLabel* FinanceReportWindow::createCard(const QString& title, QLabel** valueLabel)
{
    QLabel* card = new QLabel(this);
    card->setMinimumHeight(82);
    card->setAlignment(Qt::AlignCenter);
    card->setTextFormat(Qt::RichText);
    card->setStyleSheet(
        "QLabel { border: 1px solid rgba(148, 163, 184, 90); border-radius: 14px; padding: 10px; }"
    );

    *valueLabel = card;
    card->setText(QString("<div style='font-size:12px;'>%1</div><div style='font-size:22px; font-weight:700;'>—</div>").arg(title));
    card->setProperty("cardTitle", title);
    return card;
}

QString FinanceReportWindow::decodeValue(const QString& value) const
{
    return QString::fromUtf8(QUrl::fromPercentEncoding(value.toUtf8()).toUtf8());
}

QString FinanceReportWindow::formatMoney(double value) const
{
    QLocale locale(QLocale::Russian, QLocale::Russia);
    return locale.toString(value, 'f', 2) + QString::fromUtf8(" ₽");
}

QString FinanceReportWindow::formatPercent(double value) const
{
    QLocale locale(QLocale::Russian, QLocale::Russia);
    return locale.toString(value, 'f', 1) + "%";
}

void FinanceReportWindow::loadReport()
{
    if (startDateEdit->date() > endDateEdit->date())
    {
        QMessageBox::warning(this, QString::fromUtf8("Финансовый отчет"),
                             QString::fromUtf8("Дата начала не может быть позже даты окончания."));
        return;
    }

    statusLabel->setText(QString::fromUtf8("Формирование финансового отчета..."));

    AdminController::instance()->loadFinanceReport(
        startDateEdit->date().toString("yyyy-MM-dd"),
        endDateEdit->date().toString("yyyy-MM-dd")
    );
}

void FinanceReportWindow::handleServerMessage(const QString& message)
{
    if (!message.startsWith("FINANCE_REPORT&"))
        return;

    QStringList parts = message.split("&");
    if (parts.size() < 8)
    {
        statusLabel->setText(QString::fromUtf8("Сервер вернул некорректный отчет."));
        return;
    }

    const double planned = parts[1].toDouble();
    const double paid = parts[2].toDouble();
    const double debt = parts[3].toDouble();
    const double percent = parts[4].toDouble();
    const int activeContracts = parts[5].toInt();
    const double rentedArea = parts[6].toDouble();

    auto setCard = [](QLabel* label, const QString& title, const QString& value) {
        label->setText(QString("<div style='font-size:12px;'>%1</div><div style='font-size:22px; font-weight:700;'>%2</div>").arg(title, value));
    };

    setCard(plannedValueLabel, QString::fromUtf8("Начислено"), formatMoney(planned));
    setCard(paidValueLabel, QString::fromUtf8("Поступило"), formatMoney(paid));
    setCard(debtValueLabel, QString::fromUtf8("Задолженность"), formatMoney(debt));
    setCard(percentValueLabel, QString::fromUtf8("Оплата"), formatPercent(percent));
    setCard(activeContractsValueLabel, QString::fromUtf8("Активных договоров"), QString::number(activeContracts));
    setCard(rentedAreaValueLabel, QString::fromUtf8("Сданная площадь"), QString::number(rentedArea, 'f', 2) + QString::fromUtf8(" м²"));

    QStringList blocks = parts[7].split("|");
    if (blocks.isEmpty())
        return;

    QStringList headers = blocks[0].split(";");
    reportTable->clear();
    reportTable->setColumnCount(headers.size());
    reportTable->setHorizontalHeaderLabels(headers);
    reportTable->setRowCount(qMax(0, blocks.size() - 1));

    for (int row = 1; row < blocks.size(); ++row)
    {
        QStringList values = blocks[row].split(";");
        for (int col = 0; col < headers.size() && col < values.size(); ++col)
        {
            QString text = decodeValue(values[col]);
            QTableWidgetItem* item = new QTableWidgetItem(text);

            if (headers[col] == QString::fromUtf8("Статус"))
            {
                if (text == QString::fromUtf8("Оплачено"))
                    item->setForeground(QBrush(QColor(22, 163, 74)));
                else if (text == QString::fromUtf8("Переплата"))
                    item->setForeground(QBrush(QColor(37, 99, 235)));
                else if (text == QString::fromUtf8("Не оплачено"))
                    item->setForeground(QBrush(QColor(220, 38, 38)));
                else
                    item->setForeground(QBrush(QColor(234, 88, 12)));
            }

            reportTable->setItem(row - 1, col, item);
        }
    }

    statusLabel->setText(QString::fromUtf8("Отчет сформирован. Начисления считаются по месячной ставке аренды."));
}
