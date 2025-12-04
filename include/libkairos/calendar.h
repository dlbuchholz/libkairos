#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDate>
#include "libkairos/workday.h"
#include "libkairos/journal.h"

typedef enum TimespanUnit {
    Week,
    Month,
    Quarter,
} e_TimespanUnit;

class Calendar
{
public:
    // Function to calculate total workable hours in a month
    static double workableHours(int year, int month, double hoursPerDay, const Journal *journal = nullptr);
    static double workableHours(QDate &from, QDate &to, double hoursPerDay, const Journal *journal = nullptr);

    // Function to check if a given date is a holiday
    static bool isHoliday(const QDate &date, const Journal *journal = nullptr);

    // Runtime-konfigurierbare Zusatzfeiertage
    static void addCustomHoliday(const QDate &date);
    static void removeCustomHoliday(const QDate &date);
    static void clearCustomHolidays();

private:
    static QSet<QDate> s_customHolidays;
};

#endif // CALENDAR_H
