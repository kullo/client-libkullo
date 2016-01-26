/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/api/DateTime.h>
#include <kulloclient/util/exceptions.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

K_TEST(ApiDateTimeStatic, ctorFailsForInvalidArgs)
{
    EXPECT_ANY_THROW(Api::DateTime(2015, 13, 32, 25, 61, 61, 0));
}

K_TEST(ApiDateTimeStatic, ctorWorks)
{
    EXPECT_NO_THROW(Api::DateTime(2015, 1, 1, 0, 0, 0, 0));
}

K_TEST(ApiDateTimeStatic, nowUtcWorks)
{
    EXPECT_NO_THROW(Api::DateTime::nowUtc());
}


class ApiDateTime : public KulloTest
{
public:
    ApiDateTime()
        : uut(year, month, day,
              hour, minute, second,
              tzOffsetMinutes)
    {}

protected:
    int16_t year = 2015;
    int8_t month = 1;
    int8_t day = 2;
    int8_t hour = 3;
    int8_t minute = 4;
    int8_t second = 5;
    int16_t tzOffsetMinutes = 6;
    std::string timeString = "2015-01-02T03:04:05+00:06";
    Api::DateTime uut;
};

K_TEST_F(ApiDateTime, streamingWorks)
{
    std::stringstream result;
    result << uut;
    EXPECT_THAT(result.str(), Eq(timeString));
}

K_TEST_F(ApiDateTime, fieldsPreserveCtorArguments)
{
    EXPECT_THAT(uut.year, year);
    EXPECT_THAT(uut.month, month);
    EXPECT_THAT(uut.day, day);
    EXPECT_THAT(uut.hour, hour);
    EXPECT_THAT(uut.minute, minute);
    EXPECT_THAT(uut.second, second);
    EXPECT_THAT(uut.tzOffsetMinutes, tzOffsetMinutes);
}

K_TEST_F(ApiDateTime, operatorEqualWorks)
{
    auto same =  Api::DateTime(
                year, month, day,
                hour, minute, second,
                tzOffsetMinutes);
    EXPECT_THAT(uut, Eq(same));

    auto other =  Api::DateTime(
                year + 1, month, day,
                hour, minute, second,
                tzOffsetMinutes);
    EXPECT_THAT(uut, Not(Eq(other)));
}

K_TEST_F(ApiDateTime, operatorLessWorks)
{
    auto same =  Api::DateTime(
                year, month, day,
                hour, minute, second,
                tzOffsetMinutes);
    EXPECT_THAT(uut, Not(Lt(same)));

    auto greater =  Api::DateTime(
                year + 1, month, day,
                hour, minute, second,
                tzOffsetMinutes);
    EXPECT_THAT(uut, Lt(greater));
}
