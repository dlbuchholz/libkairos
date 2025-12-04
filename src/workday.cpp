#include "libkairos/workday.h"

Workday::Workday(QDate date) : date(date) {
    saldo = 0;
}

Workday Workday::fromJson(const QJsonObject& json) {
    Workday workday(QDate::fromString(json["date"].toString(), Qt::ISODate));

    QJsonArray segmentsArray = json["segments"].toArray();
    for (const QJsonValue& segmentValue : segmentsArray) {
        if (segmentValue.isObject()) {
            WorkdaySegment segment = WorkdaySegment::fromJson(segmentValue.toObject());
            workday.addSegment(segment);
        }
    }

    QJsonObject workedHoursInfo = json["worked_statistics"].toObject();
    workday.setTargetTime(QTime::fromString(workedHoursInfo["target_time"].toString()));
    workday.setSaldo(workedHoursInfo["saldo"].toInt());

    return workday;
}

QJsonObject Workday::toJson() const {
    QJsonObject json;

    addBasicInfoJson(json);
    addSegmentsJson(json);
    addSegmentStatisticsJson(json);
    addBreakIntervalsJson(json);
    addBreakTimeInfoJson(json);
    addWorkedHoursInfoJson(json);

    return json;
}

void Workday::addBasicInfoJson(QJsonObject& json) const {
    json["date"] = date.toString(Qt::ISODate);
    json["type"] = (getType() == PresentType) ? "Present" : "Absent";
}

void Workday::addSegmentsJson(QJsonObject& json) const {
    QJsonArray segmentsArray;
    for (const WorkdaySegment& segment : segments) {
        segmentsArray.append(segment.toJson());
    }
    json["segments"] = segmentsArray;
}

void Workday::addSegmentStatisticsJson(QJsonObject& json) const {
    QMap<WorkdaySegmentType, QTime> timePerSegmentMap = timePerSegment();
    QMap<WorkdaySegmentType, double> modalSplitAbsMap = calculateModalSplitAbs();
    QMap<WorkdaySegmentType, double> modalSplitToTargetTimeMap = calculateModalSplitToTargetTime();
    QJsonObject combinedInfoJson;
    for (auto it = timePerSegmentMap.constBegin(); it != timePerSegmentMap.constEnd(); ++it) {
        QJsonObject segmentInfoJson;
        segmentInfoJson["time"] = it.value().toString("hh:mm:ss");
        segmentInfoJson["absolute"] = modalSplitAbsMap.value(it.key());
        segmentInfoJson["percentage_to_target_time"] = modalSplitToTargetTimeMap.value(it.key());
        combinedInfoJson[WorkdaySegment::typeToString(it.key())] = segmentInfoJson;
    }
    json["segments_statistics"] = combinedInfoJson;
}

void Workday::addBreakIntervalsJson(QJsonObject& json) const {
    QList<BreakInterval> breakIntervalsList = breakIntervals();
    QJsonArray breakIntervalsArray;
    for (const auto& interval : breakIntervalsList) {
        QJsonObject intervalJson;
        intervalJson["from"] = interval.first.toString("hh:mm:ss");
        intervalJson["to"] = interval.second.toString("hh:mm:ss");
        breakIntervalsArray.append(intervalJson);
    }
    json["break_intervals"] = breakIntervalsArray;
}

void Workday::addBreakTimeInfoJson(QJsonObject& json) const {
    QJsonObject breakTimeInfo;
    breakTimeInfo["manual_break"] = manualBreakTime().toString("hh:mm:ss");
    breakTimeInfo["total_break"] = calculateDeductedBreak().toString("hh:mm:ss");
    json["break_statistics"] = breakTimeInfo;
}

void Workday::addWorkedHoursInfoJson(QJsonObject& json) const {
    QJsonObject workedHoursInfo;
    workedHoursInfo["target_time"] = targetTime.toString("hh:mm:ss");
    workedHoursInfo["total_worked"] = calculateWorkedHours().toString("hh:mm:ss");
    workedHoursInfo["net_worked"] = calculateDeductedWorkedHours().toString("hh:mm:ss");
    workedHoursInfo["worked_diff_target"] = (getWorktimeDiff() / 3600000);
    workedHoursInfo["saldo"] = (saldo / 3600000);
    json["worked_statistics"] = workedHoursInfo;
}

/* Sorts the segments on a workday by their start time. */
void Workday::sortSegmentsByStartTime() {
    std::sort(segments.begin(), segments.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.getStartTime() < rhs.getStartTime();
    });
}

WorkdayType Workday::getType() const
{
    // Iterate through each segment
    for (const auto& segment : segments) {

        if (segment.getType() == TelecommuteWork ||
            segment.getType() == AbsentFlexday ||
            segment.getType() == BusinessTrip ||
            segment.getType() == OfficeWork) {
            return WorkdayType::PresentType;
        }
    }

    return WorkdayType::AbsentType;
}

/* Returns *all* segments on a workday */
QVector<WorkdaySegment> Workday::getSegments() const
{
    return segments;
}

/* Returns the "dominant" segment type of the day for tax purposes.
 *
 * Priority:
 *   1. OfficeWork      -> first workplace visited => not a home office day
 *   2. BusinessTrip
 *   3. TelecommuteWork -> only if no OfficeWork / BusinessTrip
 *   4. Otherwise: segment type with the longest duration (if any)
 */
WorkdaySegmentType Workday::dominantSegmentTypeForTax() const
{
    bool hasOfficeWork = false;
    bool hasTelecommuteWork = false;
    bool hasBusinessTrip = false;

    // Erste Stufe: nur Präsenz-artige Segmente erfassen (Steuerlogik)
    for (const auto &segment : segments) {
        switch (segment.getType()) {
        case WorkdaySegmentType::OfficeWork:
            hasOfficeWork = true;
            break;
        case WorkdaySegmentType::TelecommuteWork:
            hasTelecommuteWork = true;
            break;
        case WorkdaySegmentType::BusinessTrip:
            hasBusinessTrip = true;
            break;
        default:
            break;
        }
    }

    // Steuerliche Prioritaet:
    if (hasOfficeWork) {
        return WorkdaySegmentType::OfficeWork;
    }
    if (hasBusinessTrip) {
        return WorkdaySegmentType::BusinessTrip;
    }
    if (hasTelecommuteWork) {
        return WorkdaySegmentType::TelecommuteWork;
    }

    // Fallback: kein steuerlich relevanter Work-Segmenttyp.
    // Nimm den Typ mit der groessten absoluten Dauer an diesem Tag.
    if (segments.isEmpty()) {
        return WorkdaySegmentType::Invalid; // Wenn der Parser nicht genau verstanden hat, um was für eine Tätigkeit es sich handelt
    }

    // Dauer pro Segmenttyp bestimmen
    QMap<WorkdaySegmentType, double> absSplit = calculateModalSplitAbs();
    WorkdaySegmentType bestType = segments.first().getType();
    double bestHours = -1.0;

    for (auto it = absSplit.constBegin(); it != absSplit.constEnd(); ++it) {
        if (it.value() > bestHours) {
            bestHours = it.value();
            bestType = it.key();
        }
    }

    return bestType;
}


void Workday::setSegments(const QVector<WorkdaySegment> &newSegments)
{
    segments = newSegments;
}

/* Add a new segment to a workday */
void Workday::addSegment(WorkdaySegment &segment)
{
    segments.append(segment); // Add the provided segment to the segments vector
}

/* Return the date of a workday */
QDate Workday::getDate() const
{
    return date;
}

/* Set the date of a workday */
void Workday::setDate(const QDate &newDate)
{
    date = newDate;
}

/* Adds together all gaps between workday segments and returns that time.
   NOTE: returned value is *NOT* adjusted for automatically deducted breaks */
QTime Workday::manualBreakTime() const {
    QTime totalBreakTime(0, 0);

    // Iterate through each segment
    for (size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments[i];

        // Add break time within a segment (e.g. break time automatically
        // deducted by ZEUS for telecommute work)
        totalBreakTime = totalBreakTime.addSecs(abs(segment.getBreakTime().secsTo(QTime(0, 0))));

        // If there is a next segment, calculate the gap and add it to the total break time
        if (i + 1 < segments.size()) {
            const auto& nextSegment = segments[i + 1];
            QTime endTimeCurrentSegment = segment.getEndTime();
            QTime startTimeNextSegment = nextSegment.getStartTime();

            // Calculate the gap between the current segment and the next segment
            int gapSeconds = endTimeCurrentSegment.secsTo(startTimeNextSegment);

            // Add the gap to the total break time only if it's at least 15 min
            if (gapSeconds >= 15 * 60) {
                totalBreakTime = totalBreakTime.addSecs(gapSeconds);
            }
        }
    }

    return totalBreakTime;
}

/* Returns a list of all break intervals */
QList<BreakInterval> Workday::breakIntervals() const {
    QList<BreakInterval> breakIntervals;

    for(size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments[i];

        // If there is a next segment, calculate the gap and add it to the total break time
        if (i + 1 < segments.size()) {
            const auto& nextSegment = segments[i + 1];
            QTime endTimeCurrentSegment = segment.getEndTime();
            QTime startTimeNextSegment = nextSegment.getStartTime();

            // Calculate the gap between the current segment and the next segment
            int gapSeconds = endTimeCurrentSegment.secsTo(startTimeNextSegment);

            // Add the gap to the total break time only if it's at least 15 min
            if (gapSeconds >= 15 * 60) {
                BreakInterval breakInterval(endTimeCurrentSegment, startTimeNextSegment);
                breakIntervals.append(breakInterval);
            }
        }
    }

    return breakIntervals;
}

/* Returns how much break time needs to be deducted automatically by specifying
 * a threshold (e.g. 9h) after which a specified  minimum break (e.g. 45min)
 * needs to get deducted.
 * Returns 00:00h if no break needs to get deducted.
 *
 * NOTE: make sure not to deduct too many automatic breaks; requires keeping
 * track of the total break time outside of this method.
 * this method, by itself, does NOT keep track of the total break time
 */
QTime Workday::breakDeduction(const QTime& workedHours,
                              const QTime& manualBreak,
                              const QTime& threshold,
                              const QTime& minimumBreak) const {
    if(workedHours < threshold || manualBreak >= minimumBreak) {
        return QTime(0, 0);
    }

    // adjustmentThreshold = threshold + minimumBreak
    QTime adjustmentThreshold = threshold.addSecs(-minimumBreak.secsTo(QTime(0, 0)));

    /* calculate the difference between the worked hours and the threshold,
     * if the worked hours are below the adjustmentThreshold (threshold + minimumBreak) */
    if(workedHours < adjustmentThreshold) {
        int diffToThreshold = abs(threshold.msecsTo(workedHours));
        return QTime::fromMSecsSinceStartOfDay(diffToThreshold);
    } else {
        QTime remainingBreak = minimumBreak.addSecs(manualBreak.secsTo(QTime(0, 0)));
        return remainingBreak;
    }
}

/* Returns the time span of all workday segments added together. */
QTime Workday::calculateWorkedHours() const {
    QTime bruttoWorkedHours(0, 0);

    // Iterate through each segment
    for (const auto& segment : segments) {
        // Calculate the duration of the segment (ignoring breaks)
        int segmentDuration = segment.getStartTime().secsTo(segment.getEndTime());

        // If a segment contains a break, it should only be deducted
        // from the worked hours if it's not an absent type
        if (segment.getType() == TelecommuteWork || segment.getType() == BusinessTrip) {
            segmentDuration -= QTime(0, 0).secsTo(segment.getBreakTime());
        }
        bruttoWorkedHours = bruttoWorkedHours.addSecs(segmentDuration);
    }

    return bruttoWorkedHours;
}

/* Returns the total break of a day by deducting manual and automatic
   break times.
 * ENSURES: manual and automatic breaks get deducted only once
 */
QTime Workday::calculateDeductedBreak() const {
    // Start with brutto worked hours
    QTime workedHours = calculateWorkedHours();
    int mSec = 0;
    QTime currentBreak = manualBreakTime();

    // automatically deduct 45min after 9h of working
    QTime threshold2 = breakDeduction(workedHours, currentBreak,
                                      QTime(9, 0), QTime(0, 45));
    currentBreak.addMSecs(threshold2.msecsSinceStartOfDay());
    mSec += threshold2.msecsSinceStartOfDay();

    // automatically deduct 30min after 6h of working
    QTime threshold1 = breakDeduction(workedHours, currentBreak,
                                      QTime(6, 0), QTime(0, 30));
    currentBreak.addMSecs(threshold1.msecsSinceStartOfDay());
    mSec += threshold1.msecsSinceStartOfDay();

    return QTime::fromMSecsSinceStartOfDay(mSec);
}

/* Returns the difference between the worked hours (breaks deducted) and the
 * target time in milliseconds.
 *
 * REQUIRES: targetTime needs to be set!
 * NOTE: Value may be negative if worked hours < target time. */
int Workday::getWorktimeDiff() const
{
    return targetTime.msecsTo(calculateDeductedWorkedHours());
}

/* Returns the work time of a day and automatically deducts manual and automatic
   break times. */
QTime Workday::calculateDeductedWorkedHours() const {
    // Start with brutto worked hours
    QTime workedHours = calculateWorkedHours();
    int workMSec = workedHours.msecsSinceStartOfDay();
    QTime currentBreak = manualBreakTime();

    // automatically deduct 45min after 9h of working
    QTime threshold2 = breakDeduction(workedHours, currentBreak,
                                      QTime(9, 0), QTime(0, 45));
    currentBreak = currentBreak.addMSecs(threshold2.msecsSinceStartOfDay());
    workMSec -= threshold2.msecsSinceStartOfDay();

    // automatically deduct 30min after 6h of working
    QTime threshold1 = breakDeduction(workedHours, currentBreak,
                                      QTime(6, 0), QTime(0, 30));
    currentBreak = currentBreak.addMSecs(threshold1.msecsSinceStartOfDay());
    workMSec -= threshold1.msecsSinceStartOfDay();

    return QTime::fromMSecsSinceStartOfDay(workMSec);
}

/* Calculate the modal split of the segment types on a workday, and return a map
   with the percentage commited to each segment type. */
QMap<WorkdaySegmentType, double> Workday::calculateModalSplit() const {
    QMap<WorkdaySegmentType, double> modalSplit;

    QTime totalWorkTime = calculateWorkedHours();
    int totalWorkSeconds = totalWorkTime.msecsSinceStartOfDay() / 1000;

    QMap<WorkdaySegmentType, int> typeWorkTime;
    for (const auto& segment : segments) {
        typeWorkTime[segment.getType()] += abs(segment.getEndTime()
                          .secsTo(segment.getStartTime()));
    }

    for (const auto& key : typeWorkTime.keys()) {
        int workSeconds = typeWorkTime[key];
        double percentage = (workSeconds
                             / static_cast<double>(totalWorkSeconds)) * 100.0;
        modalSplit[key] = percentage;
    }

    return modalSplit;
}

QTime Workday::getTargetTime() const
{
    return targetTime;
}

void Workday::setTargetTime(const QTime &newTargetTime)
{
    targetTime = newTargetTime;
}

int Workday::getSaldo() const
{
    return saldo;
}

void Workday::setSaldo(int newSaldo)
{
    saldo = newSaldo;
}

void Workday::setFZ(QString newFZ)
{
    fzShort = newFZ;
}

QString Workday::getFZ()
{
    return fzShort;
}

int Workday::getTP()
{
    return tpShort;
}

void Workday::setTP(int newTP)
{
    tpShort = newTP;
}


/* */
QMap<WorkdaySegmentType, double> Workday::calculateModalSplitToTargetTime() const {
    QMap<WorkdaySegmentType, double> modalSplit;

    int timeSeconds = targetTime.msecsSinceStartOfDay() / 1000;

    QMap<WorkdaySegmentType, int> typeWorkTime;
    for (const auto& segment : segments) {
        typeWorkTime[segment.getType()] += abs(segment.getEndTime()
                                                   .secsTo(segment.getStartTime()));
    }

    for (const auto& key : typeWorkTime.keys()) {
        int workSeconds = typeWorkTime[key];
        double percentage = (workSeconds
                             / static_cast<double>(timeSeconds)) * 100.0;
        modalSplit[key] = percentage;
    }

    return modalSplit;
}

/* Calculate the modal split of the segment types on a workday, and return a map
   with the absolute worked hours commited to each segment type. */
QMap<WorkdaySegmentType, QTime> Workday::timePerSegment() const {
    QMap<WorkdaySegmentType, QTime> modalSplit;

    for (const auto& segment : segments) {
        modalSplit[segment.getType()] = QTime(0, 0);
    }

    for (const auto& segment : segments) {
        QTime value = QTime::fromMSecsSinceStartOfDay(segment.getStartTime().msecsTo(segment.getEndTime()));
        modalSplit[segment.getType()] = modalSplit[segment.getType()].addMSecs(value.msecsSinceStartOfDay());
    }

    return modalSplit;
}

/* Calculate the modal split of the segment types on a workday, and return a map
   with the absolute worked hours commited to each segment type. */
QMap<WorkdaySegmentType, double> Workday::calculateModalSplitAbs() const {
    QMap<WorkdaySegmentType, double> modalSplit;

    for (const auto& segment : segments) {
        double workHours = (double) segment.getEndTime().secsTo(segment.getStartTime()) / -3600;
        modalSplit[segment.getType()] += workHours;
    }

    return modalSplit;
}
