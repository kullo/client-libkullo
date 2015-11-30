/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/InternalDateTimeUtils.h"

namespace Kullo {
namespace Api {

bool InternalDateTimeUtils::isValid(
        int16_t year, int8_t month, int8_t day,
        int8_t hour, int8_t minute, int8_t second,
        int16_t tzOffsetMinutes)
{
    try
    {
        DateTime(year, month, day, hour, minute, second, tzOffsetMinutes);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

DateTime InternalDateTimeUtils::nowUtc()
{
    return DateTime::nowUtc();
}

std::string InternalDateTimeUtils::toString(const DateTime &dateTime)
{
    return dateTime.toString();
}

int8_t InternalDateTimeUtils::compare(const DateTime &lhs, const DateTime &rhs)
{
    if (lhs < rhs) return -1;
    if (lhs == rhs) return 0;
    return 1;
}

}
}
