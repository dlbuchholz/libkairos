#include <gtest/gtest.h>
#define private public
#include "libkairos/journal.h"
#undef private

class JournalTest : public ::testing::Test {
	protected:
    void SetUp() override {
        journal = new Journal();
        // Populate the journal with some initial workdays
        Workday workday1(QDate(2024, 4, 20));
        WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12, 0), OfficeWork);
        WorkdaySegment segment2 = WorkdaySegment(QTime(17, 0), QTime(19, 0), TelecommuteWork);
        workday1.addSegment(segment1);
        workday1.addSegment(segment2);
        journal->addWorkday(workday1);

        Workday workday2(QDate(2024, 4, 21));
        WorkdaySegment segment3 = WorkdaySegment(QTime(9, 0), QTime(13, 0), OfficeWork);
        workday2.addSegment(segment3);
        journal->addWorkday(workday2);
	}
	
    void TearDown() override {
        delete journal;
    }

    Journal* journal;
};

TEST_F(JournalTest, GetWorkdaysInMonth) {
    // Create a Journal and add some workdays
    Journal journal;

    // Create a Journal and add some workdays
    journal.addWorkday(Workday(QDate(2022, 1, 1)));
    journal.addWorkday(Workday(QDate(2022, 1, 5)));
    journal.addWorkday(Workday(QDate(2022, 2, 2)));

    // Get workdays for January 2022
    QVector<Workday> january = journal.getWorkdaysInMonth(2022, 1);

    ASSERT_EQ(january.size(), 2);
    EXPECT_EQ(january[0].getDate(), QDate(2022, 1, 1));
    EXPECT_EQ(january[1].getDate(), QDate(2022, 1, 5));

    // Get workdays for February 2022
    QVector<Workday> february = journal.getWorkdaysInMonth(2022, 2);

    ASSERT_EQ(february.size(), 1);
    EXPECT_EQ(february[0].getDate(), QDate(2022, 2, 2));
}

TEST_F(JournalTest, GetAvailableMonths) {

    // Create new journal
    Journal journal;

    // Add the entire month of January
    for (int day = 1; day <= QDate(2022, 1,1).daysInMonth(); ++day) {
        journal.addWorkday(Workday(QDate(2022, 1, day)));
    }

    // Add an incomplete dataset of February
	journal.addWorkday(Workday(QDate(2022, 2, 2)));

    // Get available months
	QMap<QPair<int, int>, bool> availableMonths = journal.getAvailableMonths();

    // January should exist and be complete
    ASSERT_TRUE(availableMonths.contains(QPair<int, int>(2022, 1)));
    EXPECT_TRUE(availableMonths.value(QPair<int, int>(2022, 1)));

    // February should exist but be incomplete
	ASSERT_TRUE(availableMonths.contains(QPair<int, int>(2022, 2)));
	EXPECT_FALSE(availableMonths.value(QPair<int, int>(2022, 2)));

    // March should neither exist nor be complete
    ASSERT_FALSE(availableMonths.contains(QPair<int, int>(2022, 3)));
	EXPECT_FALSE(availableMonths.value(QPair<int, int>(2022, 3)));
}

TEST_F(JournalTest, GetWorkdays){

    // Create new journal
    Journal journal;

    // Add complete Month of January 2024
    for (int day = 1; day <= QDate(2024, 1, 1).daysInMonth(); ++day){
        journal.addWorkday(Workday(QDate(2024, 1, day)));
    }

    // Add complete Month of February 2024
    for (int day = 1; day <= QDate(2024, 2, 1).daysInMonth(); ++day){
        journal.addWorkday(Workday(QDate(2024, 2, day)));
    }

    // Add incomplete month of march
    journal.addWorkday(Workday(QDate(2024, 3, 1)));

    QVector<Workday> availableWorkdays = journal.getWorkdays();
    QVector<Workday> availableWorkdaysTimePeriod = journal.getWorkdays(QDate(2024, 1, 14), QDate(2024, 1, 25));

    ASSERT_FALSE(availableWorkdays.isEmpty());
    EXPECT_FALSE(availableWorkdays.isEmpty());

    ASSERT_EQ(availableWorkdays.size(), 61);
    EXPECT_EQ(availableWorkdays[1].getDate(), QDate(2024, 1, 2));
    EXPECT_EQ(availableWorkdays[42].getDate(), QDate(2024, 02, 12));

    ASSERT_EQ(availableWorkdaysTimePeriod.size(), 12);
    EXPECT_EQ(availableWorkdaysTimePeriod[6].getDate(), QDate(2024, 1, 20));
}

TEST_F(JournalTest, GetSaldo){

    // Create new journal
    Journal journal;

    // Add complete Month of January 2024
    for (int day = 1; day <= QDate(2024, 1, 1).daysInMonth(); ++day){
        journal.addWorkday(Workday(QDate(2024, 1, day)));
    }

    QVector<Workday> availableWorkdaysSaldo = journal.getWorkdays();

    ASSERT_EQ(availableWorkdaysSaldo.size(), 31);
    EXPECT_EQ(availableWorkdaysSaldo[0].getSaldo(), 0);
}

TEST_F(JournalTest, GetSize){

    // Create new journal
    Journal journal;

    // Add complete Month of January 2024
    for (int day = 1; day <= QDate(2024, 1, 1).daysInMonth(); ++day){
        journal.addWorkday(Workday(QDate(2024, 1, day)));
    }

    uint availableWorkdaysSize = journal.getSize();

    ASSERT_EQ(availableWorkdaysSize, 31);
    EXPECT_EQ(availableWorkdaysSize, 31);
}

// <<<<<<< feature/resolve-conflicts
// Test case for the merge function without a conflict
TEST_F(JournalTest, Merge) {
    // Create another journal with additional workdays for merging
    Journal otherJournal;
    Workday workday3(QDate(2024, 4, 22));
    WorkdaySegment segment3 = WorkdaySegment(QTime(10, 0), QTime(14, 0), OfficeWork);
    workday3.addSegment(segment3);
    otherJournal.addWorkday(workday3);

    // Merge the other journal into the current journal
    journal->merge(otherJournal);

    // Verify that the merged journal contains the expected workdays
    EXPECT_EQ(journal->getWorkdays().size(), 3); // Ensure correct number of workdays after merging

    // Check specific workdays after merging
    ASSERT_TRUE(journal->dateExists(QDate(2024, 4, 20))); // Check if the workday from the other journal is added
    ASSERT_TRUE(journal->dateExists(QDate(2024, 4, 21))); // Check if the existing workday is retained
    ASSERT_TRUE(journal->dateExists(QDate(2024, 4, 22))); // Check if the new workday from the other journal is added
}

// Test case for the merge function with a conflict scenario
TEST_F(JournalTest, MergeConflict) {
    // Create another journal with conflicting workdays for merging
    Journal otherJournal;
    Workday workday4(QDate(2024, 4, 21)); // Conflicting date
    WorkdaySegment segment4 = WorkdaySegment(QTime(8, 0), QTime(12, 0), OfficeWork); // Conflicting segment
    workday4.addSegment(segment4);
    otherJournal.addWorkday(workday4);

    // Merge the other journal into the current journal
    journal->merge(otherJournal);

    // Verify that the merged journal contains the expected workdays
    EXPECT_EQ(journal->getWorkdays().size(), 2); // Ensure correct number of workdays after merging

    // Check specific workdays after merging
    ASSERT_TRUE(journal->dateExists(QDate(2024, 4, 20))); // Check if the existing workday is retained
    ASSERT_TRUE(journal->dateExists(QDate(2024, 4, 21))); // Check if the conflicting workday is retained

    // Ensure that the conflicting segment from the other journal is not added to the merged journal
    Workday& conflictingWorkday = journal->getWorkdayByDate(QDate(2024, 4, 21));
    ASSERT_EQ(conflictingWorkday.getSegments().size(), 1); // Ensure only one segment exists after merging
    ASSERT_NE(conflictingWorkday.getSegments()[0], segment4); // Ensure conflicting segment is not added
}

TEST_F(JournalTest, RemoveWorkdays) {

    // Create new journal
    Journal journal;

    uint expectedSize = 0;

    // Add the entire month of January
    for (int day = 1; day <= QDate(2022, 1,1).daysInMonth(); ++day) {
        journal.addWorkday(Workday(QDate(2022, 1, day)));
    }

    QVector<Workday> workdays = journal.getWorkdaysInMonth(2022, 1);

    for(const auto &workday : workdays){
        journal.removeWorkday(workday.getDate());
    }

    EXPECT_EQ(journal.getSize(), expectedSize);
}

//=======
TEST_F(JournalTest, ToJson) {
    // Create a Journal object
    Journal journal;

    // Add a workday to the journal
    Workday workday1(QDate(2024, 4, 20));
    WorkdaySegment segment1 = WorkdaySegment(QTime(8, 0), QTime(12, 0), OfficeWork);
    WorkdaySegment segment2 = WorkdaySegment(QTime(13, 0), QTime(17, 0), TelecommuteWork);
    workday1.addSegment(segment1);
    workday1.addSegment(segment2);

    // Add another workday to the journal
    Workday workday2(QDate(2024, 4, 21));
    WorkdaySegment segment3 = WorkdaySegment(QTime(9, 0), QTime(13, 0), OfficeWork);
    workday2.addSegment(segment3);

    journal.addWorkday(workday1);
    journal.addWorkday(workday2);

    // Serialize the object to JSON
    QJsonObject json = journal.toJson();

    // Validate the JSON object
    QJsonArray workdaysArray = json["workdays"].toArray();
    ASSERT_EQ(workdaysArray.size(), 2); // Ensure correct number of workdays

    // Validate the first workday's segments
    QJsonObject workday1Json = workdaysArray[0].toObject();
    ASSERT_EQ(workday1Json["date"].toString(), "2024-04-20");
    QJsonArray segmentsArray1 = workday1Json["segments"].toArray();
    ASSERT_EQ(segmentsArray1.size(), 2); // Ensure correct number of segments for workday1

    // Validate the second workday's segments
    QJsonObject workday2Json = workdaysArray[1].toObject();
    ASSERT_EQ(workday2Json["date"].toString(), "2024-04-21");
    QJsonArray segmentsArray2 = workday2Json["segments"].toArray();
    ASSERT_EQ(segmentsArray2.size(), 1); // Ensure correct number of segments for workday2
}

TEST_F(JournalTest, ToJson_EmptyJournal) {
    // Create an empty Journal object
    Journal journal;

    // Serialize the object to JSON
    QJsonObject json = journal.toJson();

    // Validate the JSON object
    EXPECT_TRUE(json["workdays"].toArray().isEmpty());
}

// Test case for fromJson()
TEST_F(JournalTest, FromJson) {
    // Create a JSON object representing a Journal
    QJsonObject json;

    // Add some workdays to the JSON object
    QJsonArray workdaysArray;
    QJsonObject workday1, workday2;
    workday1["date"] = "2024-04-20";
    QJsonArray segmentsArray1;
    QJsonObject segment1, segment2;
    segment1["from"] = "08:00:00";
    segment1["to"] = "12:00:00";
    segment1["type"] = static_cast<int>(OfficeWork);
    segment2["from"] = "13:00:00";
    segment2["to"] = "17:00:00";
    segment2["type"] = static_cast<int>(TelecommuteWork);
    segmentsArray1.append(segment1);
    segmentsArray1.append(segment2);
    workday1["segments"] = segmentsArray1;
    workday2["date"] = "2024-04-21";
    QJsonArray segmentsArray2;
    QJsonObject segment3;
    segment3["from"] = "09:00:00";
    segment3["to"] = "13:00:00";
    segment3["type"] = static_cast<int>(OfficeWork);
    segmentsArray2.append(segment3);
    workday2["segments"] = segmentsArray2;
    workdaysArray.append(workday1);
    workdaysArray.append(workday2);
    json["workdays"] = workdaysArray;

    // Deserialize the JSON object to a Journal
    Journal journal = Journal::fromJson(json);

    // Validate the Journal object
    ASSERT_EQ(journal.getWorkdays().size(), 2); // Ensure correct number of workdays

    // Validate the first workday's segments
    const Workday& day1 = journal.getWorkdays()[0];
    ASSERT_EQ(day1.getDate(), QDate(2024, 4, 20));
    ASSERT_EQ(day1.getSegments().size(), 2); // Ensure correct number of segments for workday1

    // Validate the second workday's segments
    const Workday& day2 = journal.getWorkdays()[1];
    ASSERT_EQ(day2.getDate(), QDate(2024, 4, 21));
    ASSERT_EQ(day2.getSegments().size(), 1); // Ensure correct number of segments for workday2
}

TEST_F(JournalTest, FromJson_EmptyJson) {
    // Create an empty JSON object
    QJsonObject json;

    // Deserialize the empty JSON object to a Journal
    Journal journal = Journal::fromJson(json);

    // Validate the default values
    EXPECT_TRUE(journal.getWorkdays().isEmpty());
}

//>>>>>>> develop
