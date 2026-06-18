#include "retailspace.h"

RetailSpace::RetailSpace() : m_idSpace(0), m_floorNumber(0), m_area(0.0), m_hasAirConditioner(false), m_rentPricePerDay(0.0) {}
RetailSpace::RetailSpace(int idSpace, int floorNumber, double area, bool hasAirConditioner, double rentPricePerDay)
    : m_idSpace(idSpace), m_floorNumber(floorNumber), m_area(area), m_hasAirConditioner(hasAirConditioner), m_rentPricePerDay(rentPricePerDay) {}
int RetailSpace::idSpace() const { return m_idSpace; }
int RetailSpace::floorNumber() const { return m_floorNumber; }
double RetailSpace::area() const { return m_area; }
bool RetailSpace::hasAirConditioner() const { return m_hasAirConditioner; }
double RetailSpace::rentPricePerDay() const { return m_rentPricePerDay; }
