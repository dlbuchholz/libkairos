#ifndef TIME_H
#define TIME_H

#include <QTime>

/* Time is a custom data type to represent both a time entry and a time span
 * in Kairos. It is primarily a wrapper for QTime and helps to simplify time
 * calculations in Kairos (e.g. add, subtract, mulitiply).
 *
 * It encapsulates a duration of time in terms of seconds and
 * provides methods to convert it to and from various time representations
 * such as QTime and string format (hh:mm:ss)
 *
 * NOTE: Compared to QTime, LibKairos::Time can be negative!
 */
class Time
{
public:
    // Constructors
    Time();
    explicit Time(int seconds);
    explicit Time(const QTime& time);
    explicit Time(const QString &timeString, const QString &format = "hh:mm:ss");

    static Time abs_diff(const Time &start, const Time &end);
    static Time diff(const Time &start, const Time &end);

    int seconds() const;
    void setSeconds(int seconds);

    int hours() const;
    int minutes() const;
    int totalMinutes() const;
    int totalSeconds() const;

    // Conversion
    QTime toQTime() const;
    QString toString() const;
    QString toString(const QString &format) const;
    QString toOffsetString() const;
    QString toOffsetString(const QString &format) const;


    // Arithmetic operators
    Time operator+(const Time& other) const;
    Time operator-(const Time& other) const;
    Time operator*(const double factor) const;
private:
    int m_seconds;
};

#endif // TIME_H
