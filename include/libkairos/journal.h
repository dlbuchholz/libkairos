#ifndef JOURNAL_H
#define JOURNAL_H

#include "libkairos/workday.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>

/**
 * @brief The Journal class represents a collection of workdays.
 *
 * This class provides functionality to manage a journal containing
 * a set of workdays.
 * It allows serialization to and deserialization from JSON format.
 */
class Journal
{
public:
    /**
     * @brief Constructs a new Journal object.
     *
     * This constructor initializes an empty journal.
     */
    Journal();

    /**
     * @brief Deserialize a Journal object from JSON.
     *
     * This method deserializes a Journal object from a JSON object.
     *
     * @param json The JSON object to deserialize.
     * @return The deserialized Journal object.
     */
    static Journal fromJson(const QJsonObject &json);

    /**
     * @brief Serialize the Journal object to JSON.
     *
     * This method serializes the Journal object to a JSON object.
     *
     * @return The serialized Journal object as a JSON object.
     */
    QJsonObject toJson() const;

    /**
     * @brief Add a workday to the journal.
     *
     * This method adds a workday to the journal.
     *
     * @param workday The workday to add.
     */
    void addWorkday(const Workday &workday);

    /**
     * @brief Remove a workday from the journal by date.
     *
     * This method removes a workday from the journal based on its date.
     *
     * @param date The date of the workday to remove.
     */
    void removeWorkday(const QDate &date);

    /**
     * @brief Merge another journal into this journal.
     *
     * This method merges another journal into this journal.
     *
     * @param other The journal to merge into this journal.
     */
    void merge(const Journal &other);

    /**
     * @brief Get the size of the journal.
     *
     * This method returns the number of workdays in the journal.
     *
     * @return The size of the journal.
     */
    uint getSize() const;

    /**
     * @brief Check if the journal is empty.
     *
     * This method checks if the journal is empty (contains no workdays).
     *
     * @return True if the journal is empty, false otherwise.
     */
    bool isEmpty() const;

    /**
     * @brief Check if a workday with the given date exists in the journal.
     *
     * This method checks if a workday with the given date exists in the journal.
     *
     * @param date The date to check.
     * @return True if a workday with the given date exists, false otherwise.
     */
    bool dateExists(const QDate &date);

    /**
     * @brief Get all workdays in the journal.
     *
     * This method returns all workdays in the journal.
     *
     * @return A QVector containing all workdays in the journal.
     */
    QVector<Workday> getWorkdays() const;

    /**
     * @brief Get workdays within a specific date range.
     *
     * This method returns workdays within the specified date range.
     *
     * @param from The start date of the date range.
     * @param to The end date of the date range.
     * @return A QVector containing workdays within the specified date range.
     */
    QVector<Workday> getWorkdays(const QDate &from, const QDate &to) const;

    /**
     * @brief Get workdays within a specific month.
     *
     * This method returns workdays within the specified month and year.
     *
     * @param year The year of the month.
     * @param month The month (1 for January, 2 for February, etc.).
     * @return A QVector containing workdays within the specified month.
     */
    QVector<Workday> getWorkdaysInMonth(int year, int month) const;

    /**
     * @brief Get available months in the journal.
     *
     * This method returns a map indicating which months have workdays in the journal.
     * The keys of the map are pairs of (year, month), and the values indicate whether
     * there are workdays in the corresponding month.
     *
     * @return A QMap containing available months in the journal.
     */
    QMap<QPair<int, int>, bool> getAvailableMonths() const;


    /**
     * @brief Get the deducted worked hours data.
     *
     * This method retrieves the deducted worked hours data for the entire journal.
     *
     * @return A QVector containing the deducted worked hours data.
     */
    QVector<double> getDeductedWorkedHoursData() const;

    /**
     * @brief Get the deducted worked hours data for a specific date range.
     *
     * This method retrieves the deducted worked hours data for the specified date range.
     *
     * @param from The start date of the date range.
     * @param to The end date of the date range.
     * @return A QVector containing the deducted worked hours data for the specified date range.
     */
    QVector<double> getDeductedWorkedHoursData(const QDate &from, const QDate &to) const;

    /**
     * @brief Calculate the telecommute time between two dates.
     *
     * This method calculates the total telecommute time (in hours) between the specified dates.
     *
     * @param from The start date of the period.
     * @param to The end date of the period.
     * @return The telecommute period in hours.
     */
    double calculateTelecommutePeriod(const QDate &from, const QDate &to);

    /**
     * @brief Calculate the remaining telecommute time within a period.
     *
     * This method calculates the remaining telecommute time (in hours) within the specified period,
     * considering the given factor.
     *
     * @param from The start date of the period.
     * @param to The end date of the period.
     * @param factor The factor to consider for the calculation.
     * @return The remaining telecommute time in hours.
     */
    double remainingTelecommuteTime(const QDate &from, const QDate &to, const double factor, const double targetTime);

    /**
     * @brief Calculate the spent telecommute time within a period.
     *
     * This method calculates the spent telecommute time (in percent) within the specified period.
     *
     * @param from The start date of the period.
     * @param to The end date of the period.
     * @return The spent telecommute time in percent.
     */
    double spentTelecommuteTime(const QDate &from, const QDate &to);

    /**
     * @brief Get the saldo (balance)
     *
     * This method sums up the saldo of all workdays on the journal and returns
     * the result in hours.
     *
     * @return The saldo (balance) in hours
     */
    int getSaldo() const;

    /**
     * @brief Get the target time
     *
     * This Method is getting the last valid target time in the journal.
     * If the journal does not have a valid target time, then the default of 7,8 hr is selected.
     *
     * @return The target time as double
     */
    double getTargetTime() const;

    double getTimeValues(QString string) const;
    double getMaxTelecommuteTime(const QDate &from, const QDate &to, const double factor, const double targetTime) const;
    double effectiveWorkableHours(const QDate &from, const QDate &to, double targetTime, bool directCalculation = false) const;
private:
    mutable QVector<Workday> workdays; /**< List of workdays in the journal. */
    /**
     * @brief Sort the workdays by date.
     *
     * This method sorts the workdays in the journal by date in ascending order.
     * The sorting is performed in-place.
     */
    void sortWorkdaysByDate() const;

    /**
     * @brief Get the workday with the specified date.
     *
     * This method retrieves the workday from the journal that corresponds to the specified date.
     *
     * @param date The date of the workday to retrieve.
     * @return A reference to the workday with the specified date.
     * @throws std::runtime_error if no workday exists for the specified date.
     * @note Due to the increased possibility of crashing the application, this method is private.
     */
    Workday &getWorkdayByDate(const QDate &date);

};

#endif // JOURNAL_H
