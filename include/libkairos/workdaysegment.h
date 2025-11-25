#ifndef WORKDAYSEGMENT_H
#define WORKDAYSEGMENT_H

#include <qdatetime.h>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>

typedef enum e_WorkdaySegmentType {
    OfficeWork,
    TelecommuteWork,
    BusinessTrip,
    AbsentHoliday,
    AbsentWeekend,
    AbsentVacation,
    AbsentEducation,
    AbsentMedical,
    AbsentFlexday, // Gleittag
    AbsentExcused, // generic absent (excused)
    Absent,        // generic absent (not excused)
    Invalid,       // Only possible if an error occurs while importing journals
} WorkdaySegmentType;


class WorkdaySegment
{
public:
    bool operator!=(const WorkdaySegment &other) const;

    WorkdaySegment(QTime from, QTime to);
    WorkdaySegment(QTime from, QTime to, WorkdaySegmentType type);
    static WorkdaySegment fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    QTime getStartTime() const;
    void setStartTime(const QTime &newFrom);

    QTime getEndTime() const;
    void setEndTime(const QTime &newTo);

    WorkdaySegmentType getType() const;
    void setType(WorkdaySegmentType newType);

    QTime getBreakTime() const;
    void setBreakTime(const QTime &newBreakTime);

    static WorkdaySegmentType typeFromString(const QString &typeString);
    static QString typeToString(WorkdaySegmentType type);
    QTime getSegmentSpan() const;

private:
    QTime from;
    QTime to;
    QTime break_time = QTime(0, 0);
    QString note;
    WorkdaySegmentType type;
};

#endif // WORKDAYSEGMENT_H
