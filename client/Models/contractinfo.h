#ifndef CONTRACTINFO_H
#define CONTRACTINFO_H

#include <QString>

class ContractInfo
{
public:
    ContractInfo();
    ContractInfo(int idContract, const QString& conclusionDate, int idSpace,
                 const QString& startDate, const QString& endDate, double plannedAmount);

    int idContract() const;
    QString conclusionDate() const;
    int idSpace() const;
    QString startDate() const;
    QString endDate() const;
    double plannedAmount() const;

private:
    int m_idContract;
    QString m_conclusionDate;
    int m_idSpace;
    QString m_startDate;
    QString m_endDate;
    double m_plannedAmount;
};

#endif // CONTRACTINFO_H
