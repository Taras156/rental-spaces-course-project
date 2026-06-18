#ifndef RETAILSPACE_H
#define RETAILSPACE_H

class RetailSpace
{
public:
    RetailSpace();
    RetailSpace(int idSpace, int floorNumber, double area, bool hasAirConditioner, double rentPricePerDay);

    int idSpace() const;
    int floorNumber() const;
    double area() const;
    bool hasAirConditioner() const;
    double rentPricePerDay() const;

private:
    int m_idSpace;
    int m_floorNumber;
    double m_area;
    bool m_hasAirConditioner;
    double m_rentPricePerDay;
};

#endif // RETAILSPACE_H
