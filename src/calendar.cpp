#include "libkairos/calendar.h"

QSet<QDate> Calendar::s_customHolidays;

void Calendar::addCustomHoliday(const QDate &date)
{
    s_customHolidays.insert(date);
}

void Calendar::removeCustomHoliday(const QDate &date)
{
    s_customHolidays.remove(date);
}

void Calendar::clearCustomHolidays()
{
    s_customHolidays.clear();
}

QDate calculateEasterSunday(int year) {
    // Computus algorithm to calculate the date of Easter Sunday
    int a = year % 19;
    int b = year / 100;
    int c = year % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int month = (h + l - 7 * m + 114) / 31;
    int day = ((h + l - 7 * m + 114) % 31) + 1;
    return QDate(year, month, day);
}

/* Calculate total workable hours in a month */
double Calendar::workableHours(int year, int month,
                               double hoursPerDay, const Journal *journal) {
    int totalDays = QDate(year, month, 1).daysInMonth();
    double workableHours = 0;

    // Loop through each day in month
    for (int day = 1; day <= totalDays; ++day) {
        QDate currentDate(year, month, day);
        // Check if the current day is saturday or sunday
        if (currentDate.dayOfWeek() != Qt::Saturday &&
            currentDate.dayOfWeek() != Qt::Sunday) {
            // Check if current day is not a holiday
            if(!isHoliday(currentDate, journal)) {
                workableHours += hoursPerDay;
            }
        }
    }

    return workableHours;
}

/* Calculate total workable hours in a specified time span */
double Calendar::workableHours(QDate& from, QDate& to,
                               double hoursPerDay, const Journal *journal) {
    double workableHours = 0;

    // Loop through each day in month
    for (QDate currentDate = from; currentDate <= to; currentDate = currentDate.addDays(1)) {
        // Check if the current day is saturday or sunday
        if (currentDate.dayOfWeek() != Qt::Saturday &&
            currentDate.dayOfWeek() != Qt::Sunday) {
            // Check if current day is not a holiday
            if(!isHoliday(currentDate, journal)) {
                workableHours += hoursPerDay;
            }
        }
    }

    return workableHours;
}

/* Function to check if a given date is a holiday */
bool Calendar::isHoliday(const QDate &date, const Journal *journal)
{
    // 1. Fixed calendar holidays
    QList<QDate> holidays = {
        QDate(date.year(), 1, 1),       // New Year's Day
        QDate(date.year(), 5, 1),       // Labour Day
        QDate(date.year(), 10, 3),      // Day of German Unity
        QDate(date.year(), 12, 25),     // Christmas Day
        QDate(date.year(), 12, 26),     // Second Christmas Day
    };

    // Add movable holidays
    QDate easterSunday = calculateEasterSunday(date.year());
    holidays.append(easterSunday.addDays(-2)); // Good Friday
    holidays.append(easterSunday.addDays(1));  // Easter Monday
    holidays.append(easterSunday.addDays(39)); // Ascension Day
    holidays.append(easterSunday.addDays(50)); // Whit Monday

    if (holidays.contains(date)) {
        return true;
    }

    // 2. Custom holidays, which Kairos will specify at runtime
    for (const QDate &custom : s_customHolidays) {
        if (custom.month() == date.month() &&
            custom.day()  == date.day()) {
            return true;
        }
    }

    // 3. Optional: derive "holidays" from Journal (specific to user)
    if (!journal) {
        return false;
    }

    QVector<Workday> workdays = journal->getWorkdays();

    for (auto &workday : workdays) {
        if (workday.getDate() == date) {
            int tpShortCode = workday.getTP();
            QVector<WorkdaySegment> segments = workday.getSegments();
            for (auto &segment : segments) {
                WorkdaySegmentType type = segment.getType();
                if (type == Invalid)
                    return false;
            }
            /* Eigentlich gedacht um im JournalParser nicht erkannte Feiertage aufzufangen;
             * allerdings führt dies dazu, dass tatsächlich statt gefundene Arbeitstage als
             * Feiertag erkannt werden durch die folgende If-Abfrage
             *
             * FIXME: Mein Vorschlag ist, dass diese Abfrage hier nicht mehr gemacht wird,
             * und dass Firmenschließtage und sonstige Arbeitsfreie Tage
             * einfach unter CustomHolidays eingetragen werden
             *
             * if (tpShortCode != 101 && workday.getTargetTime() == QTime(0, 0))
                return true;*/
        }
    }

    return false;
}
