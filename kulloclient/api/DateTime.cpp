/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/DateTime.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <boost/optional.hpp>

#include "kulloclient/api/InternalDateTimeUtils.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Api {

DateTime::DateTime(
        int16_t year, int8_t month, int8_t day,
        int8_t hour, int8_t minute, int8_t second,
        int16_t tzOffsetMinutes)
    : DateTimeBase(year, month, day, hour, minute, second, tzOffsetMinutes)
    , dateTime_(year, month, day, hour, minute, second, tzOffsetMinutes * 60)
{
    if (dateTime_.isNull()) throw std::invalid_argument("DateTime");
}

boost::optional<DateTime> DateTime::fromRfc3339(const std::string &timeStr)
{
    try
    {
        return DateTime(Util::DateTime(timeStr));
    }
    catch (Util::InvalidDateTime &)
    {
        return boost::none;
    }
}

DateTime::DateTime(const Util::DateTime &dateTime)
    : DateTimeBase(
          dateTime.year(), dateTime.month(), dateTime.day(),
          dateTime.hour(), dateTime.minute(), dateTime.second(),
          std::round(dateTime.tzOffset() / 60))
    , dateTime_(dateTime)
{
    if (dateTime_.isNull()) throw std::invalid_argument("DateTime");
}

std::string DateTime::toString() const
{
    return dateTime_.toString();
}

bool DateTime::operator==(const DateTime &rhs) const
{
    return dateTime_ == rhs.dateTime_;
}

bool DateTime::operator<(const DateTime &rhs) const
{
    return dateTime_ < rhs.dateTime_;
}

DateTime operator+(const DateTime &lhs, std::chrono::seconds rhs)
{
    return DateTime(lhs.dateTime_ + rhs);
}

DateTime operator+(std::chrono::seconds lhs, const DateTime &rhs)
{
    return DateTime(lhs + rhs.dateTime_);
}

std::ostream &operator<<(std::ostream &lhs, const DateTime &rhs)
{
    lhs << rhs.dateTime_;
    return lhs;
}

DateTime DateTime::nowUtc()
{
    return DateTime(Util::DateTime::nowUtc());
}

}
}
