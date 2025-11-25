#include <gtest/gtest.h>
#define private public
#include "libkairos/workdaysegment.h"
#undef private

class WorkdaySegmentTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST(WorkdaySegmentTest, ConstructorAndGetters) {
    QTime from(9, 0);
    QTime to(17, 0);
    WorkdaySegment segment(from, to);

    EXPECT_EQ(segment.getStartTime(), from);
    EXPECT_EQ(segment.getEndTime(), to);
}

TEST(WorkdaySegmentTest, Setters) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));

    QTime newFrom(10, 0);
    QTime newTo(18, 0);
    segment.setStartTime(newFrom);
    segment.setEndTime(newTo);

    EXPECT_EQ(segment.getStartTime(), newFrom);
    EXPECT_EQ(segment.getEndTime(), newTo);
}

TEST(WorkdaySegmentTest, TypeToString) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));

    segment.setType(OfficeWork);
    EXPECT_EQ(segment.typeToString(segment.getType()), "Büroarbeit");

    segment.setType(TelecommuteWork);
    EXPECT_EQ(segment.typeToString(segment.getType()), "Flexibles Arbeiten");
}

TEST(WorkdaySegmentTest, InvalidTypeToString) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));
    segment.setType(Invalid);
    EXPECT_EQ(segment.typeToString(segment.getType()), "Unbekannt");
}

TEST(WorkdaySegmentTest, GetType) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));
    segment.setType(OfficeWork);
    EXPECT_EQ(segment.getType(), OfficeWork);
}

TEST(WorkdaySegmentTest, SetType) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));
    segment.setType(TelecommuteWork);
    EXPECT_EQ(segment.getType(), TelecommuteWork);
}

TEST(WorkdaySegmentTest, BreakTime) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));

    QTime breakTime(0, 30);
    segment.setBreakTime(breakTime);

    EXPECT_EQ(segment.getBreakTime(), breakTime);
}

TEST(WorkdaySegmentTest, SegmentSpanWithBreak) {
    WorkdaySegment segment(QTime(9, 0), QTime(17, 0));
    QTime breakTime(0, 30);
    segment.setBreakTime(breakTime);

    // Expected span: 8 hours; included break should not affect anything
    EXPECT_EQ(segment.getSegmentSpan(), QTime(8, 00));
}

TEST(WorkdaySegmentTest, NegativeSegmentSpan) {
    // Invalid segment, end before start
    WorkdaySegment segment(QTime(17, 0), QTime(9, 0));
    // Expecting a zero span
    EXPECT_EQ(segment.getSegmentSpan(), QTime(0, 0));
}

TEST(WorkdaySegmentTest, ToJson) {
    WorkdaySegment segment(QTime(8, 0), QTime(17, 0), OfficeWork);

    // Serialize the object to JSON
    QJsonObject json = segment.toJson();

    EXPECT_EQ(json["from"].toString(), "08:00:00");
    EXPECT_EQ(json["to"].toString(), "17:00:00");
    EXPECT_EQ(json["type"].toInt(), static_cast<int>(OfficeWork));
}

TEST(WorkdaySegmentTest, ToJson_EmptySegment) {
    // Create an empty WorkdaySegment object
    WorkdaySegment segment(QTime(), QTime(), Invalid);

    // Serialize the object to JSON
    QJsonObject json = segment.toJson();

    // Validate the JSON object
    EXPECT_EQ(json["from"], "");
    EXPECT_EQ(json["to"], "");
    EXPECT_EQ(json["type"], WorkdaySegment::typeToString(Invalid));
}

TEST(WorkdaySegmentTest, FromJson) {
    // Create a JSON object representing a WorkdaySegment
    QJsonObject json;
    json["from"] = "08:00:00";
    json["to"] = "17:00:00";
    json["type"] = "Büroarbeit";

    // Deserialize the JSON object to a WorkdaySegment
    WorkdaySegment segment = WorkdaySegment::fromJson(json);

    // Validate the WorkdaySegment object
    EXPECT_EQ(segment.getStartTime(), QTime(8, 0));
    EXPECT_EQ(segment.getEndTime(), QTime(17, 0));
    EXPECT_EQ(segment.getType(), OfficeWork);
}

TEST(WorkdaySegmentTest, FromJson_EmptyJson) {
    // Create an empty JSON object
    QJsonObject json;

    // Deserialize the empty JSON object to a WorkdaySegment
    WorkdaySegment segment = WorkdaySegment::fromJson(json);

    // Validate the default values
    EXPECT_EQ(segment.getStartTime(), QTime());
    EXPECT_EQ(segment.getEndTime(), QTime());
    EXPECT_EQ(segment.getType(), Invalid);
}

