#include <gtest/gtest.h>
#define private public
#include "libkairos/time.h"
#undef private

TEST(TimeConstructor, DefaultConstructor) {
    Time time;
    EXPECT_EQ(time.seconds(), 0);
}

TEST(TimeConstructor, SecondsConstructor) {
    Time time(3600); // 1 hour in seconds
    EXPECT_EQ(time.seconds(), 3600);
}

TEST(TimeConstructor, QTimeConstructor) {
    QTime qtime(1, 30, 0); // 1 hour 30 minutes
    Time time(qtime);
    EXPECT_EQ(time.seconds(), 5400);
}

TEST(TimeTest, ConstructorWithDefaultFormat_ValidTime) {
    // Valid time string in the default format (hh:mm:ss)
    QString timeString = "12:34:56";

    // Create a Time object with the valid time string
    Time time(timeString);

    // Check if the total time in seconds is calculated correctly
    // Expected result: 12 hours + 34 minutes + 56 seconds = 45296 seconds
    EXPECT_EQ(time.seconds(), 45296);
}

TEST(TimeTest, ConstructorWithCustomFormat_ValidTime) {
    // Valid time string in custom format (mm:ss)
    QString timeString = "34:56";
    QString format = "mm:ss"; // Custom format

    // Create a Time object with the valid time string and custom format
    Time time(timeString, format);

    // Check if the total time in seconds is calculated correctly
    // Expected result: 34 minutes + 56 seconds = 2096 seconds
    EXPECT_EQ(time.seconds(), 2096);
}

TEST(TimeTest, ConstructorWithDefaultFormat_InvalidTime) {
    // Invalid time string (missing seconds)
    QString timeString = "12:34";

    // Check if constructing a Time object with an invalid time string throws an exception
    EXPECT_THROW(
        {
            Time t(timeString);   // actual constructor call
        },
        std::invalid_argument);
}

TEST(TimeTest, ConstructorWithCustomFormat_InvalidTime) {
    // Invalid time string (missing minutes)
    QString timeString = "34";
    QString format = "mm:ss"; // Custom format

    EXPECT_THROW(
        {
            Time t(timeString, format);   // actual constructor call
        },
        std::invalid_argument);


    // Check if constructing a Time object with an invalid time string throws an exception
    // EXPECT_THROW(Time(timeString, format), std::invalid_argument);
}

TEST(TimeTest, ConstructorWithMoreTwoDotsInFormatAndTimeString_InvalidFormat) {
    // Invalid time string
    QString timeString = "12:34:56:78";
    QString format = "ab:cd:ef:gh"; // Custom format

    EXPECT_THROW(
        {
            Time t(timeString, format);   // actual constructor call
        },
        std::invalid_argument);


    // Check if constructing a Time object with an invalid time string throws an exception
    // EXPECT_THROW(Time(timeString, format), std::invalid_argument);
}





TEST(TimeMethods, AbsDiff) {
    Time start(3600); // 1 hours in seconds
    Time end(7200);   // 2 hours in seconds
    Time difference = Time::diff(start, end);
    EXPECT_EQ(abs(difference.seconds()), 3600); // 1 hour difference
}

TEST(TimeMethods, AbsDiffNegative) {
    Time start(7200); // 2 hours in seconds
    Time end(3600);   // 1 hours in seconds
    Time difference = Time::diff(start, end);
    EXPECT_EQ(abs(difference.seconds()), 3600); // 1 hour difference
}

TEST(TimeMethods, Diff) {
    Time start(3600); // 1 hour in seconds
    Time end(7200);   // 2 hours in seconds
    Time difference = Time::diff(start, end);
    EXPECT_EQ(difference.seconds(), 3600); // 1 hour difference
}

TEST(TimeMethods, DiffNegative) {
    Time start(7200); // 2 hours in seconds
    Time end(3600);   // 1 hour in seconds
    Time difference = Time::diff(start, end);
    EXPECT_EQ(difference.seconds(), -3600); // Negative 1 hour difference
}

TEST(TimeMethods, Getters) {
    Time time(3660); // 1 hour and 1 minute in seconds
    EXPECT_EQ(time.seconds(), 3660);
    EXPECT_EQ(time.hours(), 1);
    EXPECT_EQ(time.minutes(), 1);
    EXPECT_EQ(time.totalMinutes(), 61);
    EXPECT_EQ(time.totalSeconds(), 3660);
}

TEST(TimeMethods, GettersNegative) {
    Time time(-3660); // 1 hour and 1 minute in seconds
    EXPECT_EQ(time.seconds(), -3660);
    EXPECT_EQ(time.hours(), -1);
    EXPECT_EQ(time.minutes(), -1);
    EXPECT_EQ(time.totalMinutes(), -61);
    EXPECT_EQ(time.totalSeconds(), -3660);
}

TEST(TimeMethods, Conversion) {
    Time time(3660); // 1 hour and 1 minute in seconds
    QTime qtime = time.toQTime();
    EXPECT_EQ(qtime.hour(), 1);
    EXPECT_EQ(qtime.minute(), 1);
    EXPECT_EQ(qtime.second(), 0);
    EXPECT_EQ(time.toString(), "01:01:00");
}

TEST(TimeOperators, Addition) {
    Time time1(3600); // 1 hour in seconds
    Time time2(1800); // 30 minutes in seconds
    Time sum = time1 + time2;
    EXPECT_EQ(sum.seconds(), 5400); // 1 hour 30 minutes
}

TEST(TimeOperators, Subtraction) {
    Time time1(7200); // 2 hours in seconds
    Time time2(3600); // 1 hour in seconds
    Time diff = time1 - time2;
    EXPECT_EQ(diff.seconds(), 3600); // 1 hour difference
}

TEST(TimeOperators, Multiplication) {
    Time time(7200); // 2 hours in seconds
    double factor = 0.1; // 10%
    Time product = time * factor;
    EXPECT_EQ(product.seconds(), 720); // 10% of 2h, i.e. 12min
}

TEST(TimeTest, ToStringWithDefaultFormat) {
    Time time(3661); // 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toString(), "01:01:01");
}

TEST(TimeTest, ToStringWithDefaultFormatAndNegative) {
    Time time(-3661); // Negative: 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toString(), "-01:01:01");
}

TEST(TimeTest, ToStringWithCustomFormat) {
    Time time(3661); // 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toString("hh'h'mm'm'ss's'"), "01h01m01s");
}

TEST(TimeTest, ToOffsetStringWithDefaultFormat) {
    Time time(3661); // 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toOffsetString(), "+01:01:01");
}

TEST(TimeTest, ToOffsetStringWithCustomFormat) {
    Time time(3661); // 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toOffsetString("hh'h'mm'm'ss's'"), "+01h01m01s");
}

TEST(TimeTest, ToStringWithCustomFormatNegative) {
    Time time(-3661); // Negative: 1 hour, 1 minute, 1 second
    EXPECT_EQ(time.toString("hh'h'mm'm'ss's'"), "-01h01m01s");
}

