#ifndef WORKDAY_H
#define WORKDAY_H

#include "workdaysegment.h"
#include <QDate>
#include <QPair>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>

using BreakInterval = QPair<QTime, QTime>;

typedef enum WorkdayType {
    PresentType,
    AbsentType,
} e_WorkdayType;

class Workday
{
public:
    Workday(QDate date);
    static Workday fromJson(const QJsonObject &json);

    QJsonObject toJson() const;

    QVector<WorkdaySegment> getSegments() const;
    void setSegments(const QVector<WorkdaySegment> &newSegments);
    // Method to add a WorkdaySegment
    void addSegment(WorkdaySegment &segment);
    QDate getDate() const;
    void setDate(const QDate &newDate);
    void sortSegmentsByStartTime();
    WorkdaySegmentType dominantSegmentTypeForTax() const;

    WorkdayType getType() const;

    QList<BreakInterval> breakIntervals() const;
    QTime manualBreakTime() const;
    QTime calculateDeductedWorkedHours() const;
    QTime calculateWorkedHours() const;
    QTime calculateDeductedBreak() const;

    int getWorktimeDiff() const;
    int getSaldo() const;

    QMap<WorkdaySegmentType, QTime> timePerSegment() const;
    QMap<WorkdaySegmentType, double> calculateModalSplitAbs() const;
    QMap<WorkdaySegmentType, double> calculateModalSplit() const;
    QMap<WorkdaySegmentType, double> calculateModalSplitToTargetTime() const;
    QTime getTargetTime() const;
    void setTargetTime(const QTime &newTargetTime);

    void setSaldo(int newSaldo);

    QString getFZ();
    void setFZ(QString newFZ);
    int getTP();
    void setTP(int newTP);

private:
    QDate date;
    QTime targetTime;
    int saldo; /* saldo in msecs. Why???, why not just int? like in the csv-file? */
    QString fzShort;
    int tpShort;
    QVector<WorkdaySegment> segments;

    QTime breakDeduction(const QTime &workedHours, const QTime &manualBreak, const QTime &threshold,
                         const QTime &minimumBreak) const;
    void addBasicInfoJson(QJsonObject &json) const;
    void addSegmentsJson(QJsonObject &json) const;
    void addSegmentStatisticsJson(QJsonObject &json) const;
    void addBreakIntervalsJson(QJsonObject &json) const;
    void addBreakTimeInfoJson(QJsonObject &json) const;
    void addWorkedHoursInfoJson(QJsonObject &json) const;
};

#endif // WORKDAY_H
