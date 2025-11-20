#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDate>
#include "libkairos/workday.h"

typedef enum TimespanUnit {
    Week,
    Month,
    Quarter,
} e_TimespanUnit;

class Calendar
{
public:
    // Function to calculate total workable hours in a month
    static double workableHours(int year, int month, double hoursPerDay);
    static double workableHours(QDate &from, QDate &to, double hoursPerDay);
private:
    // Function to check if a given date is a holiday
    static bool isHoliday(QDate &date);

};

#endif // CALENDAR_H
