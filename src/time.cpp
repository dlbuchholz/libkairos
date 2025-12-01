#include "libkairos/time.h"

#include <QString>

// Default constructor initializing seconds to 0
Time::Time() : m_seconds(0) {}

// Constructor accepting seconds as input
Time::Time(int seconds) : m_seconds(seconds) {}

// Constructor accepting a QTime object and converting it to seconds
Time::Time(const QTime& time) : m_seconds(time.hour() * 3600 + time.minute() * 60 + time.second()) {}

// Constructor that takes a QString as input
// REQUIRES: string needs to be in "hh:mm:ss" format, if no format is specified
Time::Time(const QString& timeString, const QString& format) {

    // Structural check: compare colon count
    const QString timeStringCopy = timeString;
    const int inputColons = timeStringCopy.count(':');
    const int formatColons = format.count(':');
    QString tmpStringHour = "";
    QString tmpStringMinute = "";
    QString tmpStringSecond = "";
    int counterTimeFragments = 0;

    if (timeStringCopy.length() > 0) {
        tmpStringHour = timeStringCopy.section(':', 0, 0);
        tmpStringMinute = timeStringCopy.section(':', 1, 1);
        tmpStringSecond = timeStringCopy.section(':', 2, 2);
    } else if (timeStringCopy.length() > 8 || timeStringCopy.length() <= 0) {
        throw std::invalid_argument(&"Invalid time String length. Length: " [timeStringCopy.toStdString().size()]);
    } else if (inputColons != formatColons) {
        throw std::invalid_argument("Invalid time format");
    } else if (formatColons > 2) {
        throw std::invalid_argument("Invalid format");
    } else if (format.contains('h') && (tmpStringHour.toInt() < 0 || tmpStringHour.toInt() > 25)) {
        throw std::invalid_argument("Invalid hour value " + tmpStringHour.toStdString());
    } else if (format.contains('m') && (tmpStringMinute.toInt() < 0 || tmpStringMinute.toInt() >= 60)) {
        throw std::invalid_argument("Invalid minute value " + tmpStringMinute.toStdString());
    } else if (format.contains('s') && (tmpStringSecond.toInt() < 0 || tmpStringSecond.toInt() >= 60)) {
        throw std::invalid_argument("Invalid seconds value " + tmpStringMinute.toStdString());
    } else if (format.contains('h')) {
        counterTimeFragments++;
    } else if (format.contains('m')) {
        counterTimeFragments++;
    } else if (format.contains('s')) {
        counterTimeFragments++;
    }

    for (int i = 0; i < counterTimeFragments; ++i){ // 0 -> hh, 1 -> mm, 2 -> ss
        if (timeStringCopy.contains(':')) {
            QString tmpString = timeStringCopy.section(':', i, i);
            if (tmpString.isNull()) {
                throw std::invalid_argument("String value is null " + tmpString.toStdString());
            }
            else if (tmpString.isEmpty()) {
                throw std::invalid_argument("String is empty");
            }
        } else {
            if (timeStringCopy.length() > 0 && !(timeStringCopy.isEmpty())) {
                throw std::invalid_argument("String contains invalid time format " + timeStringCopy.toStdString() + " -> should be hh:mm:ss");
            }
        }
    }

    // Parse the QString using QTime::fromString with the specified format
    QTime time = QTime::fromString(timeString, format);

    // Require valid time and exact round trip
    if (!time.isValid() || time.toString(format) != timeString) {
        throw std::invalid_argument("Invalid time format");
    }

    // Calculate the total time in seconds
    m_seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
}

/* Calculate the time difference between two Time objects and construct a new
 * Time object from the result. Result may be negative */
Time Time::diff(const Time& start, const Time& end) {
    int differenceInSeconds = end.seconds() - start.seconds();
    return Time(differenceInSeconds);
}

// Getter for seconds
int Time::seconds() const {
    return m_seconds;
}

// Setter for seconds
void Time::setSeconds(int seconds) {
    m_seconds = seconds;
}

// Calculate hours from seconds
int Time::hours() const {
    return m_seconds / 3600;
}

// Calculate minutes from seconds
int Time::minutes() const {
    return (m_seconds % 3600) / 60;
}

// Calculate total minutes from seconds
int Time::totalMinutes() const {
    return m_seconds / 60;
}

// Return total seconds
int Time::totalSeconds() const {
    return m_seconds;
}

/* Convert Time object to QTime
 *
 * NOTE: Can ONLY construct a QTime with absolute values!
 */
QTime Time::toQTime() const {
    int hours = abs(m_seconds) / 3600;
    int minutes = (abs(m_seconds) % 3600) / 60;
    int seconds = abs(m_seconds) % 60;
    return QTime(hours, minutes, seconds);
}

// Convert Time object to string format "hh:mm:ss"
QString Time::toString() const {
    // Call the other toString method with showSign = false and default format
    return toString("hh:mm:ss");
}

// Convert Time object to string with specified format
QString Time::toString(const QString& format) const {
    QTime time = toQTime();
    QString timeString = time.toString(format);
    if (m_seconds < 0) {
        timeString.prepend('-');
    }
    return timeString;
}

// Convert Time object to offset string format:
// if positive: "+hh:mm:ss"
// if negative: "-hh:mm:ss"
QString Time::toOffsetString() const {
    return toOffsetString("hh:mm:ss");
}

// Convert Time object to offset string with specified format
// if positive: "+[format]"
// if negative: "-[format]"
QString Time::toOffsetString(const QString& format) const {
    QTime time = toQTime();
    QString timeString = time.toString(format);
    if (m_seconds < 0) {
        timeString.prepend('-');
    } else {
        timeString.prepend('+');
    }
    return timeString;
}

// Overloaded addition operator to add two Time objects
Time Time::operator+(const Time& other) const {
    return Time(m_seconds + other.m_seconds);
}

// Overloaded subtraction operator to subtract two Time objects
Time Time::operator-(const Time& other) const {
    return Time(m_seconds - other.m_seconds);
}

// Overloaded multiplication operator to multiply a Time object with a double
Time Time::operator*(const double factor) const {
    return Time(m_seconds * factor);
}
