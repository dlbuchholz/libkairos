#include "libkairos/calendar.h"
//#include <config.h>
#include "libkairos/journal.h"
//#include "../applicationcontroller.h"

Journal::Journal()
{

}

Journal Journal::fromJson(const QJsonObject& json) {
    Journal journal;

    QJsonArray workdaysArray = json["workdays"].toArray();
    for (const QJsonValue& workdayValue : workdaysArray) {
        if (workdayValue.isObject()) {
            Workday workday = Workday::fromJson(workdayValue.toObject());
            journal.workdays.append(workday);
        }
    }

    return journal;
}

QJsonObject Journal::toJson() const {
    QJsonObject json;

    //json["kairos_version"] = PROJECT_VERSION;

    QJsonArray workdaysArray;
    for (const Workday& workday : workdays) {
        workdaysArray.append(workday.toJson());
    }
    json["workdays"] = workdaysArray;

    return json;
}

/* Add a workday to the journal by passing a workday */
void Journal::addWorkday(const Workday &workday)
{
    workdays.append(workday);
}

Workday& Journal::getWorkdayByDate(const QDate& date) {
    auto it = std::find_if(workdays.begin(), workdays.end(), [&](const Workday& workday) {
        return workday.getDate() == date;
    });

    if (it != workdays.end()) {
        return *it;
    } else {
        // If no workday with the provided date is found, throw an exception
        throw std::runtime_error("Workday with the provided date not found.");
    }
}

/* Returns the total telecommute time in a specified time span (in hours) */
double Journal::calculateTelecommutePeriod(const QDate& from, const QDate& to) {
    QVector<Workday> workdaysInTimespan = getWorkdays(from, to);
    double telecommutePeriod = 0;
    for (const auto& workday : workdaysInTimespan) {
        telecommutePeriod += workday.calculateModalSplitAbs().value(TelecommuteWork, 0.0);
    }
    return telecommutePeriod;
}

/* Returns the absolute remaining telecommute time (in hours) */
double Journal::remainingTelecommuteTime(const QDate& from, const QDate& to,
                                         const double factor,
                                         const double targetTime) {
    double maxTelecommuteTime = getMaxTelecommuteTime(from, to, factor, targetTime);
    return maxTelecommuteTime - calculateTelecommutePeriod(from, to);
}

double Journal::getMaxTelecommuteTime(const QDate& from, const QDate& to,
                                      const double factor,
                                      const double targetTime) {
    // Basis: alle grundsätzlich möglichen Telearbeitsstunden im Monat
    double maxTelecommuteTime =
        Calendar::workableHours(from.year(), from.month(), targetTime, this) * factor;

    // Relevante Workdays im gewünschten Zeitraum
    QVector<Workday> workdaysInTimespan = getWorkdays(from, to);

    for (const auto &workday : workdaysInTimespan) {
        const QDate date = workday.getDate();

        // Nur reguläre Werktage, die auch in workableHours enthalten sind
        if (date.dayOfWeek() == Qt::Saturday || date.dayOfWeek() == Qt::Sunday)
            continue;
        if (Calendar::isHoliday(date, this))
            continue;

        // Wenn dieser Tag AbsentType ist, gibt es an diesem Tag 0 telecommute Kapazität
        if (workday.getType() == WorkdayType::AbsentType) {
            maxTelecommuteTime -= targetTime * factor;
        }
    }

    if (maxTelecommuteTime < 0.0)
        maxTelecommuteTime = 0.0;

    return maxTelecommuteTime;
}

/* Returns the percentage of spent telecommute time */
double Journal::spentTelecommuteTime(const QDate& from, const QDate& to) {
    double workableHours = Calendar::workableHours(from.year(), from.month(), 7.8, this);
    return (calculateTelecommutePeriod(from, to) / workableHours);
}

/* Remove a workday from the journal by specifying a date */
void Journal::removeWorkday(const QDate &date)
{
    workdays.erase(std::remove_if(workdays.begin(), workdays.end(),
                                  [date](const Workday &wd)
                                  { return wd.getDate() == date; }),
                   workdays.end());
}

/* Sorts the workdays in a journal by ascending order of date */
void Journal::sortWorkdaysByDate() const {
    // Make a copy of workdays and sort it
    QVector<Workday> sortedWorkdays = workdays;  // Make a copy
    std::sort(sortedWorkdays.begin(), sortedWorkdays.end(), [](const Workday &a, const Workday &b) {
        return a.getDate() < b.getDate();
    });

    // Update the original workdays with the sorted copy
    workdays = sortedWorkdays;
}

void Journal::merge(const Journal& other) {
    // Retrieve the workdays from the other journal
    QVector<Workday> otherWorkdays = other.getWorkdays();

    // Iterate over each workday in the other journal
    for (const auto& otherWorkday : otherWorkdays) {
        // Check if the workday's date already exists in the current journal
        if (!dateExists(otherWorkday.getDate())) {
            // If the date doesn't exist, simply add the workday to the current journal
            addWorkday(otherWorkday);
        } else {
            // If the date exists, find the corresponding workday in the current journal
            Workday& currentWorkday = getWorkdayByDate(otherWorkday.getDate());

            // Compare the number of segments and the total worked hours between the workdays
            if (otherWorkday.getSegments().size() > currentWorkday.getSegments().size() ||
                otherWorkday.calculateWorkedHours() > currentWorkday.calculateWorkedHours()) {
                // If the other workday has more segments or more worked hours, replace the current workday
                currentWorkday = otherWorkday;
            }
        }
    }
}

/* Returns true if a workday with the specified date exists on the journal. */
bool Journal::dateExists(const QDate &date) {
    for(const auto& workday : workdays) {
        if (workday.getDate() == date) {
            return true;
        }
    }

    return false;
}

/* Return a set of *all* workdays on a journal. */
QVector<Workday> Journal::getWorkdays() const
{
    sortWorkdaysByDate();
    return workdays;
}

/* Return a set of workdays on a journal within a specified time frame. */
QVector<Workday> Journal::getWorkdays(const QDate& from, const QDate& to) const {
    QVector<Workday> resultWorkdays;

    sortWorkdaysByDate();

    for (const auto& workday : workdays) {
        if(workday.getDate() >= from && workday.getDate() <= to) {
            resultWorkdays.push_back(workday);
        }
    }

    return resultWorkdays;
}

/* Retrieve workdays within a specified month.
   NOTE: Will return an empty Vector if the specified month does not exist
   on the journal. */
QVector<Workday> Journal::getWorkdaysInMonth(int year, int month) const {
    QVector<Workday> result;

    for (const auto &workday : workdays) {
        QDate date = workday.getDate();

        if (date.year() == year && date.month() == month) {
            result.append(workday);
        }
    }

    return result;
}

/* Retrieve (year, month) pairs that are available on the journal, and return
   a map where that pair is the key and the value is whether that month's
   dataset is complete (i.e. *all* days of the month exist on the journal)*/
QMap<QPair<int, int>, bool> Journal::getAvailableMonths() const {
    QMap<QPair<int, int>, bool> availableMonths;

    for(const auto &workday : workdays) {
        QDate date = workday.getDate();
        int year = date.year();
        int month = date.month();

        QPair<int, int> yearMonthPair(year, month);

        // If the pair is not already present, add it with completeness set to true
        if (!availableMonths.contains(yearMonthPair)) {
            availableMonths[yearMonthPair] = true;
        }
    }

    // Check completeness by iterating over each month
    for (const auto &yearMonthPair : availableMonths.keys()) {
        int year = yearMonthPair.first;
        int month = yearMonthPair.second;

        int daysInMonth = QDate(year, month, 1).daysInMonth();

        // If the number of workdays for the month is less than the expected
        // days, set completeness to false
        if (getWorkdaysInMonth(year, month).size() < daysInMonth) {
            availableMonths[yearMonthPair] = false;
        }
    }

    return availableMonths;
}

/* Return the work time (break time deducted) of all workdays on the journal */
QVector<double> Journal::getDeductedWorkedHoursData() const
{
    QVector<double> deductedWorkedHoursData;

    for (const auto &workday : workdays) {
        QTime deductedWorkedHours = workday.calculateDeductedWorkedHours();
        deductedWorkedHoursData.append(deductedWorkedHours.hour() + deductedWorkedHours.minute() / 60.0);
    }

    return deductedWorkedHoursData;
}

/* Return the work time (break time deducted) of workdays on the journal within
   a specified time span. */
QVector<double> Journal::getDeductedWorkedHoursData(const QDate &from, const QDate &to) const
{
    QVector<double> deductedWorkedHoursData;

    for (const auto &workday : workdays) {
        if(workday.getDate() >= from && workday.getDate() <= to) {
            QTime deductedWorkedHours = workday.calculateDeductedWorkedHours();
            deductedWorkedHoursData.append(deductedWorkedHours.hour() + deductedWorkedHours.minute() / 60.0);
        }
    }

    return deductedWorkedHoursData;
}

int Journal::getSaldo() const
{
    if(isEmpty()) {
        return 0;
    }

    sortWorkdaysByDate();
    return workdays.last().getSaldo();
}

/* Returns the amount of workdays on the journal */
uint Journal::getSize() const
{
    return workdays.size();
}

double Journal::getTargetTime() const
{
    sortWorkdaysByDate();
    QTime validTime = QTime(0, 0);
    QList<QTime> targetTimes = {};

    for(const auto &workday : workdays) {
        validTime = workday.getTargetTime();
        targetTimes.append(validTime);
    }

    for(int i = 1; i < targetTimes.length(); i++) {
        if(targetTimes[i - 1] > targetTimes[i])
            validTime = targetTimes[i - 1];
        if(targetTimes[i - 1] < targetTimes[i])
            validTime = targetTimes[i];
    }
    if(validTime < targetTimes[targetTimes.length() - 1])
        validTime = targetTimes[targetTimes.length() - 1];

    if (!validTime.isValid())
        return 0.0;

    int hour = validTime.hour();
    int minute = validTime.minute();

    double targetTime = (minute / 60.0) + hour;

    return targetTime;
}


/*
 * Purpose:
 *    Converts a time string in the format "hh:mm" into a decimal hour value.
 *
 * Input:
 *    string  A time value, expected in the format "hours:minutes".
 *            Example: "08:30".
 *
 * Process:
 *    1. Trims leading and trailing whitespace.
 *    2. Splits the string at the colon into hour and minute components.
 *    3. Verifies that both components exist and can be interpreted as integers.
 *    4. Validates the numeric ranges:
 *         Hours must be between 0 and 12.
 *         Minutes must be between 0 and 59.
 *       If validation fails, the function returns 0.0.
 *    5. Computes the decimal hour result:
 *         result = hour + (minute / 60).
 *
 * Output:
 *    double  Decimal hour value.
 *            Example: "1:30" yields 1.5.
 *            Invalid input yields 0.0.
 */
double Journal::getTimeValues(QString string) const
{
    QString timeString = string.trimmed();
    /*if (logger)
        logger->log(Logger::LogLevel::Debug, QString("string: %1").arg(timeString));*/

    QStringList timeStringList = timeString.split(":");
    if (timeStringList.size() < 2 || timeStringList[0].isEmpty() || timeStringList[1].isEmpty()) {
        /*if (logger)
            logger->log(Logger::LogLevel::Debug,
                        QString("stringList: %1").arg(timeStringList.value(0)));*/
        return 0.0;
    }

    /*if (logger)
        logger->log(Logger::LogLevel::Debug,
                    QString("stringList: %1 %2").arg(timeStringList[0], timeStringList[1]));*/

    int hour = timeStringList[0].toInt();
    int minute = timeStringList[1].toInt();

    if (hour < 0 || hour >= 13 && minute >= 0 || minute < 0 || minute >= 60) {
        /*if (logger)
            logger->log(Logger::LogLevel::Warning,
                        QString("Values are not correct: hour = %1, minute = %2")
                            .arg(hour)
                            .arg(minute));*/
        return 0.0;
    }

    QTime time(hour, minute);
    return double(time.hour()) + double((time.minute() / 0.6) / 100);
}


/* Returns true if there are no workdays on the journal */
bool Journal::isEmpty() const
{
    return workdays.isEmpty();
}
