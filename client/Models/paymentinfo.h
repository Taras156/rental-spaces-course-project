#ifndef PAYMENTINFO_H
#define PAYMENTINFO_H

#include <QString>

class PaymentInfo
{
public:
    PaymentInfo();
    PaymentInfo(int idContract, int idSpace, const QString& paymentDate, double paymentAmount);

    int idContract() const;
    int idSpace() const;
    QString paymentDate() const;
    double paymentAmount() const;

private:
    int m_idContract;
    int m_idSpace;
    QString m_paymentDate;
    double m_paymentAmount;
};

#endif // PAYMENTINFO_H
