/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/assert.h>
#include <kulloclient/util/datetime.h>
#include <kulloclient/util/exceptions.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class DateTime : public KulloTest
{
};

K_TEST_F(DateTime, empty)
{
    EXPECT_THROW(Util::DateTime(""), Util::EmptyDateTime);
}

K_TEST_F(DateTime, conversionFromIntegersIsCorrect)
{
    Util::DateTime validUntil(2024, 1, 2, 15, 16, 17);
    EXPECT_THAT(validUntil.toString(), Eq("2024-01-02T15:16:17+00:00"));
}

K_TEST_F(DateTime, wrongNumberOfTs)
{
    EXPECT_THROW(Util::DateTime("2024-01-01 00:00:00Z"), Util::InvalidDateTime);
    EXPECT_THROW(Util::DateTime("2024-01-01:00:00:00TZ"), Util::InvalidDateTime);
    EXPECT_THROW(Util::DateTime("2024-01-01T00:00:00TZ"), Util::InvalidDateTime);
    EXPECT_THROW(Util::DateTime("2024-01-01T00:00:00ZT"), Util::InvalidDateTime);
}

K_TEST_F(DateTime, equalsIgnoreMilliseconds)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Eq(Util::DateTime("2024-01-01T10:35:36.000Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Eq(Util::DateTime("2024-01-01T10:35:36.132Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Eq(Util::DateTime("2024-01-01T10:35:36.5Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36.22Z"), Eq(Util::DateTime("2024-01-01T10:35:36.22Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36.22Z"), Eq(Util::DateTime("2024-01-01T10:35:36.23Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36.22Z"), Eq(Util::DateTime("2024-01-01T10:35:36.99Z")));
}

K_TEST_F(DateTime, equalsParseTimezone)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T00:00:00+00:00"), Eq(Util::DateTime("2024-01-01T00:00:00Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T20:15:33+00:00"), Eq(Util::DateTime("2024-01-01T20:15:33Z")));
}

K_TEST_F(DateTime, equalsDifferentTimezone)
{
    // full hours
    EXPECT_THAT(Util::DateTime("2024-01-01T19:30:04+02:00"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T18:30:04+01:00"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T16:30:04-01:00"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T15:30:04-02:00"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));

    // broken hours
    EXPECT_THAT(Util::DateTime("2024-01-01T19:45:04+02:15"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T20:00:04+02:30"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
    EXPECT_THAT(Util::DateTime("2024-01-01T20:15:04+02:45"), Eq(Util::DateTime("2024-01-01T17:30:04Z")));
}

K_TEST_F(DateTime, unequal)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:37Z"), Ne(Util::DateTime("2024-01-01T10:35:36Z"))); // seconds
    EXPECT_THAT(Util::DateTime("2024-01-01T10:36:01Z"), Ne(Util::DateTime("2024-01-01T10:35:36Z"))); // minutes
    EXPECT_THAT(Util::DateTime("2024-01-01T11:01:01Z"), Ne(Util::DateTime("2024-01-01T10:35:36Z"))); // hours
    EXPECT_THAT(Util::DateTime("2024-01-02T01:01:01Z"), Ne(Util::DateTime("2024-01-01T10:35:36Z"))); // days
    EXPECT_THAT(Util::DateTime("2024-09-01T01:01:01Z"), Ne(Util::DateTime("2024-01-10T10:35:36Z"))); // months
    EXPECT_THAT(Util::DateTime("2025-01-01T01:01:01Z"), Ne(Util::DateTime("2024-12-10T10:35:36Z"))); // years
}

K_TEST_F(DateTime, invalidTimezone)
{
    EXPECT_THROW(Util::DateTime("2024-01-01T00:00:00"), Util::InvalidDateTime);
}

K_TEST_F(DateTime, lowerCaseChars)
{
    EXPECT_NO_THROW(Util::DateTime("2024-01-01t00:00:00+00:00"));
    EXPECT_NO_THROW(Util::DateTime("2024-01-01T00:00:00z"));
    EXPECT_THAT(Util::DateTime("2024-01-01T00:00:00z"), Eq(Util::DateTime("2024-01-01T00:00:00Z")));
}

K_TEST_F(DateTime, lessThan)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Lt(Util::DateTime("2024-01-01T10:35:37Z"))); // seconds
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Lt(Util::DateTime("2024-01-01T10:36:01Z"))); // minutes
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Lt(Util::DateTime("2024-01-01T11:01:01Z"))); // hours
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Lt(Util::DateTime("2024-01-02T01:01:01Z"))); // days
    EXPECT_THAT(Util::DateTime("2024-01-10T10:35:36Z"), Lt(Util::DateTime("2024-09-01T01:01:01Z"))); // months
    EXPECT_THAT(Util::DateTime("2024-12-10T10:35:36Z"), Lt(Util::DateTime("2025-01-01T01:01:01Z"))); // years
}

K_TEST_F(DateTime, lessOrEqualThan)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Le(Util::DateTime("2024-01-01T10:35:36Z"))); // equal
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Le(Util::DateTime("2024-01-01T10:35:37Z"))); // seconds
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Le(Util::DateTime("2024-01-01T10:36:01Z"))); // minutes
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Le(Util::DateTime("2024-01-01T11:01:01Z"))); // hours
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z"), Le(Util::DateTime("2024-01-02T01:01:01Z"))); // days
    EXPECT_THAT(Util::DateTime("2024-01-10T10:35:36Z"), Le(Util::DateTime("2024-09-01T01:01:01Z"))); // months
    EXPECT_THAT(Util::DateTime("2024-12-10T10:35:36Z"), Le(Util::DateTime("2025-01-01T01:01:01Z"))); // years
}

K_TEST_F(DateTime, greaterThan)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:37Z"), Gt(Util::DateTime("2024-01-01T10:35:36Z"))); // seconds
    EXPECT_THAT(Util::DateTime("2024-01-01T10:36:01Z"), Gt(Util::DateTime("2024-01-01T10:35:36Z"))); // minutes
    EXPECT_THAT(Util::DateTime("2024-01-01T11:01:01Z"), Gt(Util::DateTime("2024-01-01T10:35:36Z"))); // hours
    EXPECT_THAT(Util::DateTime("2024-01-02T01:01:01Z"), Gt(Util::DateTime("2024-01-01T10:35:36Z"))); // days
    EXPECT_THAT(Util::DateTime("2024-09-01T01:01:01Z"), Gt(Util::DateTime("2024-01-10T10:35:36Z"))); // months
    EXPECT_THAT(Util::DateTime("2025-01-01T01:01:01Z"), Gt(Util::DateTime("2024-12-10T10:35:36Z"))); // years
}

K_TEST_F(DateTime, greaterOrEqualThan)
{
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:37Z"), Ge(Util::DateTime("2024-01-01T10:35:37Z"))); // equal
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:37Z"), Ge(Util::DateTime("2024-01-01T10:35:36Z"))); // seconds
    EXPECT_THAT(Util::DateTime("2024-01-01T10:36:01Z"), Ge(Util::DateTime("2024-01-01T10:35:36Z"))); // minutes
    EXPECT_THAT(Util::DateTime("2024-01-01T11:01:01Z"), Ge(Util::DateTime("2024-01-01T10:35:36Z"))); // hours
    EXPECT_THAT(Util::DateTime("2024-01-02T01:01:01Z"), Ge(Util::DateTime("2024-01-01T10:35:36Z"))); // days
    EXPECT_THAT(Util::DateTime("2024-09-01T01:01:01Z"), Ge(Util::DateTime("2024-01-10T10:35:36Z"))); // months
    EXPECT_THAT(Util::DateTime("2025-01-01T01:01:01Z"), Ge(Util::DateTime("2024-12-10T10:35:36Z"))); // years
}

K_TEST_F(DateTime, operatorPlus)
{
    Util::DateTime base("2024-01-01T10:35:14Z");
    EXPECT_THAT(base + std::chrono::seconds(  0), Eq(base));
    EXPECT_THAT(base + std::chrono::seconds(  1), Eq(Util::DateTime("2024-01-01T10:35:15Z")));
    EXPECT_THAT(base + std::chrono::seconds( -1), Eq(Util::DateTime("2024-01-01T10:35:13Z")));
    EXPECT_THAT(base + std::chrono::seconds( 10), Eq(Util::DateTime("2024-01-01T10:35:24Z")));
    EXPECT_THAT(base + std::chrono::seconds(-10), Eq(Util::DateTime("2024-01-01T10:35:04Z")));
    EXPECT_THAT(std::chrono::seconds(  0) + base, Eq(base));
    EXPECT_THAT(std::chrono::seconds(  1) + base, Eq(Util::DateTime("2024-01-01T10:35:15Z")));
    EXPECT_THAT(std::chrono::seconds( -1) + base, Eq(Util::DateTime("2024-01-01T10:35:13Z")));
    EXPECT_THAT(std::chrono::seconds( 10) + base, Eq(Util::DateTime("2024-01-01T10:35:24Z")));
    EXPECT_THAT(std::chrono::seconds(-10) + base, Eq(Util::DateTime("2024-01-01T10:35:04Z")));
}

K_TEST_F(DateTime, operatorMinusDuration)
{
    Util::DateTime base("2024-01-01T10:35:14Z");
    EXPECT_THAT(base - std::chrono::seconds(  0), Eq(base));
    EXPECT_THAT(base - std::chrono::seconds(  1), Eq(Util::DateTime("2024-01-01T10:35:13Z")));
    EXPECT_THAT(base - std::chrono::seconds( -1), Eq(Util::DateTime("2024-01-01T10:35:15Z")));
    EXPECT_THAT(base - std::chrono::seconds( 10), Eq(Util::DateTime("2024-01-01T10:35:04Z")));
    EXPECT_THAT(base - std::chrono::seconds(-10), Eq(Util::DateTime("2024-01-01T10:35:24Z")));
}

K_TEST_F(DateTime, operatorMinusDateTime)
{
    // time
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:14Z") - Util::DateTime("2024-01-01T10:35:13Z"),
                Eq(std::chrono::seconds(1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:13Z") - Util::DateTime("2024-01-01T10:35:14Z"),
                Eq(std::chrono::seconds(-1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:36:00Z") - Util::DateTime("2024-01-01T10:35:00Z"),
                Eq(std::chrono::minutes(1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:00Z") - Util::DateTime("2024-01-01T10:36:00Z"),
                Eq(std::chrono::minutes(-1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T11:00:00Z") - Util::DateTime("2024-01-01T10:00:00Z"),
                Eq(std::chrono::hours(1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:00:00Z") - Util::DateTime("2024-01-01T11:00:00Z"),
                Eq(std::chrono::hours(-1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T11:36:14Z") - Util::DateTime("2024-01-01T10:35:13Z"),
                Eq(std::chrono::hours(1) + std::chrono::minutes(1) + std::chrono::seconds(1)));
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:13Z") - Util::DateTime("2024-01-01T11:36:14Z"),
                Eq(-(std::chrono::hours(1) + std::chrono::minutes(1) + std::chrono::seconds(1))));

    // date
    EXPECT_THAT(Util::DateTime("2024-01-02T00:00:00Z") - Util::DateTime("2024-01-01T00:00:00Z"),
                Eq(std::chrono::hours(24)));
    EXPECT_THAT(Util::DateTime("2024-01-01T00:00:00Z") - Util::DateTime("2024-01-02T00:00:00Z"),
                Eq(std::chrono::hours(-24)));

    // timezone
    EXPECT_THAT(Util::DateTime("2024-01-01T10:35:36Z") - Util::DateTime("2024-01-01T12:35:36+02:00"),
              Eq(std::chrono::seconds(0)));
}
