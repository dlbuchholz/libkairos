#include "libkairos/calendar.h"
//#include "applicationcontroller.h"

/* Calculate total workable hours in a month */
double Calendar::workableHours(int year, int month,
                               double hoursPerDay) {
    int totalDays = QDate(year, month, 1).daysInMonth();
    double workableHours = 0;

    // Loop through each day in month
    for (int day = 1; day <= totalDays; ++day) {
        QDate currentDate(year, month, day);
        // Check if the current day is saturday or sunday
        if (currentDate.dayOfWeek() != Qt::Saturday &&
            currentDate.dayOfWeek() != Qt::Sunday) {
            // Check if current day is not a holiday
            if(!isHoliday(currentDate)) {
                workableHours += hoursPerDay;
            }
        }
    }

    return workableHours;
}

/* Calculate total workable hours in a specified time span */
double Calendar::workableHours(QDate& from, QDate& to,
                               double hoursPerDay) {
    double workableHours = 0;

    // Loop through each day in month
    for (QDate currentDate = from; currentDate <= to; currentDate = currentDate.addDays(1)) {
        // Check if the current day is saturday or sunday
        if (currentDate.dayOfWeek() != Qt::Saturday &&
            currentDate.dayOfWeek() != Qt::Sunday) {
            // Check if current day is not a holiday
            if(!isHoliday(currentDate)) {
                workableHours += hoursPerDay;
            }
        }
    }

    return workableHours;
}

/* Function to check if a given date is a holiday */
bool Calendar::isHoliday(QDate &date) {
    /*ApplicationController *app = ApplicationController::get_instance();
    Journal journal = app->getJournal();
    QVector<Workday> workdays = journal.getWorkdays();

    // This solution isn't the best one.
    // Probably something like get Workday(QDate &date) should be implemented, but I already used so much time on this
    for(auto& workday : workdays){
        if(workday.getDate() == date){
            int tpShortCode = workday.getTP();
            QVector<WorkdaySegment> segments = workday.getSegments();
            for(auto& segment : segments){
                WorkdaySegmentType type = segment.getType();
                if(type == Invalid)
                    return false;
            }
            // WHY is target time a QTime datatype? Isn't it just an integer
            if(tpShortCode != 101 && workday.getTargetTime() == QTime(0, 0))
                return true;
        }
    }*/

    QList<QDate> holidays = { // Only dates with a fixed date should be here...
        QDate(date.year(), 1, 1),       // New Year's Day
        QDate(date.year(), 5, 1),       // Labour Day
        QDate(date.year(), 10, 3),      // Day of German Unity
        QDate(date.year(), 12, 25),     // Christmas Day
        QDate(date.year(), 12, 26),     // Second Christmas Day
    };
    return holidays.contains(date);
}
