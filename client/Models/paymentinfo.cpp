#include "paymentinfo.h"

PaymentInfo::PaymentInfo() : m_idContract(0), m_idSpace(0), m_paymentAmount(0.0) {}
PaymentInfo::PaymentInfo(int idContract, int idSpace, const QString& paymentDate, double paymentAmount)
    : m_idContract(idContract), m_idSpace(idSpace), m_paymentDate(paymentDate), m_paymentAmount(paymentAmount) {}
int PaymentInfo::idContract() const { return m_idContract; }
int PaymentInfo::idSpace() const { return m_idSpace; }
QString PaymentInfo::paymentDate() const { return m_paymentDate; }
double PaymentInfo::paymentAmount() const { return m_paymentAmount; }
