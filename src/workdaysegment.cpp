#include "libkairos/workdaysegment.h"

bool WorkdaySegment::operator!=(const WorkdaySegment& other) const {
    // Compare the properties of the two segments
    return !(from == other.from && to == other.to && type == other.type);
}

WorkdaySegment::WorkdaySegment(QTime from, QTime to) :
    from(from), to(to)
{
    type = WorkdaySegmentType::Invalid;
}

WorkdaySegment::WorkdaySegment(QTime from, QTime to, WorkdaySegmentType type) :
    from(from), to(to), type(type) {}

WorkdaySegment WorkdaySegment::fromJson(const QJsonObject& json) {
    if (json.isEmpty()) {
        // If the JSON object is empty, return a default WorkdaySegment with invalid type
        return WorkdaySegment(QTime(), QTime(), WorkdaySegmentType::Invalid);
    }

    WorkdaySegment segment(
        QTime::fromString(json["from"].toString(), Qt::ISODate),
        QTime::fromString(json["to"].toString(), Qt::ISODate),
        typeFromString(json["type"].toString())
        );

    segment.setBreakTime(QTime::fromString(json["break_time"].toString(), Qt::ISODate));
    //segment.setNote(json["note"].toString());

    return segment;
}

QJsonObject WorkdaySegment::toJson() const
{
    QJsonObject json;
    json["from"] = from.toString(Qt::ISODate);
    json["to"] = to.toString(Qt::ISODate);
    json["break_time"] = break_time.toString(Qt::ISODate);
    json["note"] = note;
    json["type"] = typeToString(type);
    return json;
}

WorkdaySegmentType WorkdaySegment::typeFromString(const QString& typeString) {
    if (typeString == "Büroarbeit") return WorkdaySegmentType::OfficeWork;
    else if (typeString == "Flexibles Arbeiten") return WorkdaySegmentType::TelecommuteWork;
    else if (typeString == "Dienstreise") return WorkdaySegmentType::BusinessTrip;
    else if (typeString == "Wochenende") return WorkdaySegmentType::AbsentWeekend;
    else if (typeString == "Feiertag") return WorkdaySegmentType::AbsentHoliday;
    else if (typeString == "Urlaub") return WorkdaySegmentType::AbsentVacation;
    else if (typeString == "Abwesend (Unterricht)") return WorkdaySegmentType::AbsentEducation;
    else if (typeString == "Abwesend (Krankheit)") return WorkdaySegmentType::AbsentMedical;
    else if (typeString == "Gleittag") return WorkdaySegmentType::AbsentFlexday;
    else if (typeString == "Abwesend (entschuldigt)") return WorkdaySegmentType::AbsentExcused;
    else if (typeString == "Abwesend") return WorkdaySegmentType::Absent;
    else return WorkdaySegmentType::Invalid;
}

/* Represent a workday segment as a string and return that string.
   TO-DO: Should be language agnostic */
QString WorkdaySegment::typeToString(WorkdaySegmentType type) {
    switch (type) {
        case OfficeWork: return "Büroarbeit";
        case TelecommuteWork: return "Flexibles Arbeiten";
        case BusinessTrip: return "Dienstreise";
        case AbsentWeekend: return "Wochenende";
        case AbsentHoliday: return "Feiertag";
        case AbsentVacation: return "Urlaub";
        case AbsentEducation: return "Abwesend (Unterricht)";
        case AbsentMedical: return "Abwesend (Krankheit)";
        case AbsentFlexday: return "Gleittag";
        case AbsentExcused: return "Abwesend (entschuldigt)";
        case Absent: return "Abwesend";
        case Invalid: return "Unbekannt";
        default: return "";
    }
}

/* Return the start time of a workday segment. */
QTime WorkdaySegment::getStartTime() const
{
    return from;
}

/* Set the start time of a workday segment. */
void WorkdaySegment::setStartTime(const QTime &newFrom)
{
    from = newFrom;
}

/* Return the end time of a workday segment. */
QTime WorkdaySegment::getEndTime() const
{
    return to;
}

/* Return the time span between the start and end time of a workday segment */
QTime WorkdaySegment::getSegmentSpan() const {
    int milliseconds = from.msecsTo(to);
    if (milliseconds < 0) {
        // If the time span is negative, return zero
        return QTime(0, 0);
    } else {
        // Otherwise, calculate the time span normally
        return QTime(0, 0).addMSecs(milliseconds);
    }
}

/* Set the end time of a workday segment. */
void WorkdaySegment::setEndTime(const QTime &newTo)
{
    to = newTo;
}

/* Return the type of a workday segment */
WorkdaySegmentType WorkdaySegment::getType() const
{
    return type;
}

/* Set the type of a workday segment */
void WorkdaySegment::setType(WorkdaySegmentType newType)
{
    type = newType;
}

/* Return the break time that is included in a workday segment.
 * NOTE: Required for TelecommuteWork that automatically deducts breaks. */
QTime WorkdaySegment::getBreakTime() const
{
    return break_time;
}

/* Set the break time that is included in a workday segment.
 * NOTE: Required for TelecommuteWork that automatically deducts breaks. */
void WorkdaySegment::setBreakTime(const QTime &newBreakTime)
{
    break_time = newBreakTime;
}
