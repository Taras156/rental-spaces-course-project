#include "contractinfo.h"

ContractInfo::ContractInfo() : m_idContract(0), m_idSpace(0), m_plannedAmount(0.0) {}
ContractInfo::ContractInfo(int idContract, const QString& conclusionDate, int idSpace,
                           const QString& startDate, const QString& endDate, double plannedAmount)
    : m_idContract(idContract), m_conclusionDate(conclusionDate), m_idSpace(idSpace),
      m_startDate(startDate), m_endDate(endDate), m_plannedAmount(plannedAmount) {}
int ContractInfo::idContract() const { return m_idContract; }
QString ContractInfo::conclusionDate() const { return m_conclusionDate; }
int ContractInfo::idSpace() const { return m_idSpace; }
QString ContractInfo::startDate() const { return m_startDate; }
QString ContractInfo::endDate() const { return m_endDate; }
double ContractInfo::plannedAmount() const { return m_plannedAmount; }
