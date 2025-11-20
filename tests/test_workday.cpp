#include <gtest/gtest.h>
#define private public
#include "libkairos/workday.h"
#undef private

class WorkdayTest : public ::testing::Test {
	protected:
    void SetUp() override {
        workday = new Workday(QDate(2023, 1, 1));
	}
	
    void TearDown() override {
        delete workday;
    }

    Workday* workday;
};

TEST_F(WorkdayTest, ManualBreakTime) {

    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12,0));   // 4 hours
    WorkdaySegment segment2 = WorkdaySegment(QTime(12, 20), QTime(17,0)); // 4 hours, 40min

    workday->addSegment(segment1);
    workday->addSegment(segment2);

    EXPECT_EQ(workday->manualBreakTime(), QTime(0, 20));
}

TEST_F(WorkdayTest, CalculateWorkedHours) {
    
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12,0));   // 4 hours
    WorkdaySegment segment2 = WorkdaySegment(QTime(12, 20), QTime(17,0)); // 4 hours, 40min

    workday->addSegment(segment1);
    workday->addSegment(segment2);

    EXPECT_EQ(workday->calculateWorkedHours(), QTime(8, 40));
}

TEST_F(WorkdayTest, CalculateWorkedHours2) {

    WorkdaySegment segment1 = WorkdaySegment(QTime(6, 05), QTime(15,37));

    workday->addSegment(segment1);

    EXPECT_EQ(workday->calculateWorkedHours().secsTo(QTime(0, 0)), QTime(9, 32).secsTo(QTime(0, 0)));
}

TEST_F(WorkdayTest, BreakDeductionTest) {

    // Assume the employee worked for a total of 8h and 40min
    QTime nettoWorkedHours = QTime(8, 40);

    // Assume the employee took a 20min break
    QTime breakTime = QTime(0, 20);

    // The manual 20min is not enough to satisfy the minimum requirements,
    // therefore an extra 10min have to be subtracted.
    QTime result1 = workday->breakDeduction(nettoWorkedHours, breakTime,
                                 QTime(6, 0), QTime(0, 30));
    EXPECT_EQ(result1, QTime(0, 10));
    breakTime = breakTime.addMSecs(result1.msecsSinceStartOfDay());

    // The 9h threshold has not been reached, therefore it should not affect
    // break time.
    QTime result2 = workday->breakDeduction(nettoWorkedHours, breakTime,
                                 QTime(9, 0), QTime(0, 45));
    EXPECT_EQ(result2, QTime(0, 0));

    // Sum of all breaks should equal 30min now
    EXPECT_EQ(breakTime, QTime(0, 30));
}

TEST_F(WorkdayTest, BreakDeductionTest_JustAboveThreshold) {

    // Assume the employee worked for a total of 6h and 2min
    QTime workedHours = QTime(6, 02);

    // Assume the employee took no break
    QTime breakTime = QTime(0, 0);

    QTime result1 = workday->breakDeduction(workedHours, breakTime,
                                          QTime(6, 0), QTime(0, 30));
    qDebug() << breakTime.toString();
    EXPECT_EQ(result1, QTime(0, 02));
    breakTime = breakTime.addMSecs(result1.msecsSinceStartOfDay());

    // The 9h threshold has not been reached, therefore it should not affect
    // anything.
    QTime result2 = workday->breakDeduction(workedHours, breakTime,
                                          QTime(9, 0), QTime(0, 45));
    EXPECT_EQ(result2, QTime(0, 0));

    // Sum of all breaks should equal 2min now, since the 2min do not get
    // counted towards the work time
    EXPECT_EQ(breakTime, QTime(0, 02));
}

TEST_F(WorkdayTest, BreakDeductionTest_JustAboveThresholdWithManualBreak) {

    // Assume the employee worked for a total of 6h and 15min
    QTime workedHours = QTime(6, 15);

    // Assume a manual 5min break outside of the work hours accounted for
    QTime breakTime = QTime(0, 15);

    QTime result1 = workday->breakDeduction(workedHours, breakTime,
                                            QTime(6, 0), QTime(0, 30));

    // Expect a 15min break deduction
    EXPECT_EQ(result1, QTime(0, 15));
    breakTime = breakTime.addMSecs(result1.msecsSinceStartOfDay());

    // breakTime sum should equal 30min, therefore automatic deduction should
    // equal 0
    QTime result2 = workday->breakDeduction(workedHours, breakTime,
                                            QTime(9, 0), QTime(0, 45));
    EXPECT_EQ(result2, QTime(0, 0));

    // Expect breakTime sum to equal 30
    EXPECT_EQ(breakTime, QTime(0, 30));
}

TEST_F(WorkdayTest, BreakDeductionTest_JustAboveThresholdWithManualBreak2) {

    // Assume the employee worked for a total of 9h and 15min
    QTime workedHours = QTime(9, 15);

    // Assume a manual 5min break outside of the work hours accounted for
    QTime breakTime = QTime(0, 15);

    QTime result1 = workday->breakDeduction(workedHours, breakTime,
                                         QTime(6, 0), QTime(0, 30));

    // Expect a 15min break deduction
    EXPECT_EQ(result1, QTime(0, 15));
    breakTime = breakTime.addMSecs(result1.msecsSinceStartOfDay());

    // breakTime sum should equal 30min, therefore automatic deduction should
    // equal 0
    QTime result2 = workday->breakDeduction(workedHours, breakTime,
                                         QTime(9, 0), QTime(0, 45));
    EXPECT_EQ(result2, QTime(0, 15));
    breakTime = breakTime.addMSecs(result2.msecsSinceStartOfDay());

    // Expect breakTime sum to equal 30
    EXPECT_EQ(breakTime, QTime(0, 45));
}

TEST_F(WorkdayTest, CalculateDeductedWorkedHours) {
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12,0));   // 4 hours
    WorkdaySegment segment2 = WorkdaySegment(QTime(12, 20), QTime(17,0)); // 4 hours, 40min

    workday->addSegment(segment1);
    workday->addSegment(segment2);

    EXPECT_EQ(workday->calculateDeductedWorkedHours().secsTo(QTime(0, 0)), QTime(8, 30).secsTo(QTime(0, 0)));
}

TEST_F(WorkdayTest, CalculateDeductedWorkedHours2) {

    WorkdaySegment segment1 = WorkdaySegment(QTime(6, 05), QTime(15,37));   // 9:32

    workday->addSegment(segment1);

    EXPECT_EQ(workday->calculateDeductedWorkedHours().secsTo(QTime(0, 0)), QTime(9, 00).secsTo(QTime(0, 0)));
}

TEST_F(WorkdayTest, CalculateDeductedWorkedHours_WithManualBreak) {

    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(14,0));   // 6 hours
    WorkdaySegment segment2 = WorkdaySegment(QTime(15, 0), QTime(17,0)); // 2 hours

    workday->addSegment(segment1);
    workday->addSegment(segment2);

    EXPECT_EQ(workday->calculateDeductedWorkedHours(), QTime(8, 0));
}

TEST_F(WorkdayTest, CalculateDeductedWorkedHours_Weekend) {
    WorkdaySegment segment1 = WorkdaySegment(QTime(0, 0), QTime(7, 48));
    segment1.break_time = QTime(0, 30);
    segment1.setType(AbsentWeekend);
    workday->addSegment(segment1);

    EXPECT_EQ(workday->calculateDeductedWorkedHours(), QTime(7, 48));
}

TEST_F(WorkdayTest, GetWorktimeDiffTest) {
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12,0));   // 4 hours
    WorkdaySegment segment2 = WorkdaySegment(QTime(12, 20), QTime(17,0)); // 4 hours, 40min

    workday->addSegment(segment1);
    workday->addSegment(segment2);
    workday->setTargetTime(QTime(7, 48));

    int timeDiff = workday->getWorktimeDiff();

    /* 8:30 - 7:48 should be +42min */
    EXPECT_EQ(timeDiff, QTime(0, 42).msecsSinceStartOfDay());
}

TEST_F(WorkdayTest, GetWorktimeDiffTest_Negative) {
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12,0));   // 4 hours

    workday->addSegment(segment1);
    workday->setTargetTime(QTime(7, 48));

    int timeDiff = workday->getWorktimeDiff();

    /* 4 - 7:48 should be -3:48min */
    EXPECT_EQ(timeDiff, -QTime(3,48).msecsSinceStartOfDay());
}

TEST_F(WorkdayTest, SetSaldo){
    WorkdaySegment segment = WorkdaySegment(QTime(0, 0), QTime(7, 48));
    segment.break_time = QTime(0, 30);
    segment.setType(OfficeWork);
    workday->addSegment(segment);
    workday->setSaldo(5);

    ASSERT_EQ(workday->getSaldo(), 5);
}

TEST_F(WorkdayTest, CalculateModalSplit) {
    /* Add a nonsensical but theoretically valid modal split */
    WorkdaySegment segment1 = WorkdaySegment(QTime(6, 0), QTime(8, 0), OfficeWork);
    WorkdaySegment segment2 = WorkdaySegment(QTime(8, 0), QTime(10, 0), AbsentMedical);
    WorkdaySegment segment3 = WorkdaySegment(QTime(10, 0), QTime(13, 0), AbsentFlexday);
    WorkdaySegment segment4 = WorkdaySegment(QTime(13, 0), QTime(16, 0), OfficeWork);

    workday->addSegment(segment1);
    workday->addSegment(segment2);
    workday->addSegment(segment3);
    workday->addSegment(segment4);

    QMap<WorkdaySegmentType, double> modalSplit = workday->calculateModalSplit();

    // Expected modal split percentages
    QMap<WorkdaySegmentType, double> expectedModalSplit = {
        {OfficeWork, 50.0},
        {AbsentFlexday, 30.0},
        {AbsentMedical, 20.0}
    };

    // Compare each segment type's modal split percentage
    for (const auto& key : modalSplit.keys()) {
        EXPECT_DOUBLE_EQ(modalSplit[key], expectedModalSplit[key]);
    }
}

TEST_F(WorkdayTest, CalculateModalSplitAbs) {
    /* Add a nonsensical but theoretically valid modal split */
    WorkdaySegment segment1 = WorkdaySegment(QTime(6, 0), QTime(8, 0), OfficeWork);
    WorkdaySegment segment2 = WorkdaySegment(QTime(8, 0), QTime(10, 0), AbsentMedical);
    WorkdaySegment segment3 = WorkdaySegment(QTime(10, 0), QTime(13, 0), AbsentFlexday);
    WorkdaySegment segment4 = WorkdaySegment(QTime(13, 0), QTime(16, 0), OfficeWork);

    workday->addSegment(segment1);
    workday->addSegment(segment2);
    workday->addSegment(segment3);
    workday->addSegment(segment4);

    QMap<WorkdaySegmentType, double> modalSplit = workday->calculateModalSplitAbs();

    // Expected absolute worked hours per segment type
    QMap<WorkdaySegmentType, double> expectedModalSplit = {
        {OfficeWork, 5},
        {AbsentFlexday, 3},
        {AbsentMedical, 2}
    };

    // Compare each segment type's absolute worked hours
    for (const auto& key : modalSplit.keys()) {
        EXPECT_DOUBLE_EQ(modalSplit[key], expectedModalSplit[key]);
    }
}

TEST_F(WorkdayTest, FromJson) {
    // Create a JSON object representing a Workday
    QJsonObject json;

    // Add basic info
    json["date"] = "2024-04-20";
    json["type"] = "Present";

    // Add segments to the JSON object
    QJsonArray segmentsArray;
    QJsonObject segment1, segment2;
    segment1["from"] = "08:00:00";
    segment1["to"] = "12:00:00";
    segment1["type"] = WorkdaySegment::typeToString(OfficeWork);
    segment2["from"] = "13:00:00";
    segment2["to"] = "17:00:00";
    segment2["type"] = WorkdaySegment::typeToString(TelecommuteWork);
    segmentsArray.append(segment1);
    segmentsArray.append(segment2);
    json["segments"] = segmentsArray;

    // Add segment statistics
    QJsonObject segmentStats;
    QJsonObject büroarbeitStats;
    büroarbeitStats["time"] = "04:00:00";
    büroarbeitStats["absolute"] = 4.0;
    büroarbeitStats["percentage_to_target_time"] = 50.0;
    segmentStats[QString::fromUtf8("Büroarbeit")] = büroarbeitStats;
    json["segments_statistics"] = segmentStats;

    // Add break intervals
    QJsonArray breakIntervalsArray;
    QJsonObject interval1, interval2;
    interval1["from"] = "12:00:00";
    interval1["to"] = "13:00:00";
    interval2["from"] = "17:00:00";
    interval2["to"] = "18:00:00";
    breakIntervalsArray.append(interval1);
    breakIntervalsArray.append(interval2);
    json["break_intervals"] = breakIntervalsArray;

    // Add break time info
    QJsonObject breakTimeInfo;
    breakTimeInfo["manual_break"] = "01:00:00";
    breakTimeInfo["total_break"] = "02:00:00";
    json["break_statistics"] = breakTimeInfo;

    // Add worked hours info
    QJsonObject workedHoursInfo;
    workedHoursInfo["target_time"] = "07:48:00";
    workedHoursInfo["total_worked"] = "08:00:00";
    workedHoursInfo["net_worked"] = "07:00:00";
    workedHoursInfo["worked_diff_target"] = "01:00:00";
    workedHoursInfo["saldo"] = 7.0;
    json["worked_statistics"] = workedHoursInfo;

    // Deserialize the JSON object to a Workday
    Workday workday = Workday::fromJson(json);

    // Validate the Workday object
    ASSERT_EQ(workday.getDate(), QDate(2024, 4, 20));
    ASSERT_EQ(workday.getType(), PresentType);
    ASSERT_EQ(workday.getSegments().size(), 2); // Ensure correct number of segments
    ASSERT_EQ(workday.getSaldo(), 7.0);
    ASSERT_EQ(workday.getTargetTime(), QTime::fromString("07:48:00"));

    // Validate the first segment
    const WorkdaySegment& segment1Deserialized = workday.getSegments()[0];
    EXPECT_EQ(segment1Deserialized.getStartTime(), QTime(8, 0));
    EXPECT_EQ(segment1Deserialized.getEndTime(), QTime(12, 0));
    EXPECT_EQ(segment1Deserialized.getType(), OfficeWork);

    // Validate the second segment
    const WorkdaySegment& segment2Deserialized = workday.getSegments()[1];
    EXPECT_EQ(segment2Deserialized.getStartTime(), QTime(13, 0));
    EXPECT_EQ(segment2Deserialized.getEndTime(), QTime(17, 0));
    EXPECT_EQ(segment2Deserialized.getType(), TelecommuteWork);
}

TEST_F(WorkdayTest, FromJson_EmptyJson) {
    // Create an empty JSON object
    QJsonObject json;

    // Deserialize the empty JSON object to a Workday
    Workday workday = Workday::fromJson(json);

    // Validate the default values
    EXPECT_EQ(workday.getDate(), QDate());
    EXPECT_TRUE(workday.getSegments().isEmpty());
}

TEST_F(WorkdayTest, ToJson) {
    // Create a Workday object
    Workday workday(QDate(2024, 4, 20));

    // Add some segments to the workday
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12, 0), OfficeWork);
    WorkdaySegment segment2 = WorkdaySegment(QTime(13, 0), QTime(17, 0), TelecommuteWork);
    workday.addSegment(segment1);
    workday.addSegment(segment2);

    // Serialize the object to JSON
    QJsonObject json = workday.toJson();

    // Validate the JSON object
    EXPECT_EQ(json["date"].toString(), "2024-04-20");
    QJsonArray segmentsArray = json["segments"].toArray();
    ASSERT_EQ(segmentsArray.size(), 2); // Ensure correct number of segments

    // Validate the first segment
    QJsonObject segment1Json = segmentsArray[0].toObject();
    EXPECT_EQ(segment1Json["from"].toString(), "08:00:00");
    EXPECT_EQ(segment1Json["to"].toString(), "12:00:00");
    EXPECT_EQ(segment1Json["type"].toString(), WorkdaySegment::typeToString(OfficeWork));

    // Validate the second segment
    QJsonObject segment2Json = segmentsArray[1].toObject();
    EXPECT_EQ(segment2Json["from"].toString(), "13:00:00");
    EXPECT_EQ(segment2Json["to"].toString(), "17:00:00");
    EXPECT_EQ(segment2Json["type"].toString(), WorkdaySegment::typeToString(TelecommuteWork));
}

TEST_F(WorkdayTest, ToJson_EmptyWorkday) {
    // Create an empty Workday object
    Workday workday(QDate(0, 0, 0));

    // Serialize the object to JSON
    QJsonObject json = workday.toJson();

    // Validate the JSON object
    EXPECT_EQ(json["date"], "");
    EXPECT_TRUE(json["segments"].toArray().isEmpty());
}
