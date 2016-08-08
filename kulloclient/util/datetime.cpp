/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/datetime.h"

#include <cstdlib>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/numeric_cast.h"

namespace Kullo {
namespace Util {

namespace {
const boost::regex RFC3339_REGEX(
        /* yyyy-mm-ddT     */ R"#((\d{4})-(\d{2})-(\d{2})T)#"
        /* hh:mm:ss[.ffff] */ R"#((\d{2}):(\d{2}):(\d{2})(?:\.\d+)?)#"
        /* Z|(+|-)hh:mm    */ R"#((?:Z|(\+|-)(\d{2}):(\d{2})))#",
        boost::regex::perl | boost::regex::icase);
}

struct DateTime::Impl
{
    const boost::posix_time::ptime dateTime_;
    const boost::posix_time::minutes tzOffset_{0};

    Impl() {}

    Impl(const boost::posix_time::ptime &dateTime, const boost::posix_time::minutes &tzOffset)
        : dateTime_(dateTime),
          tzOffset_(tzOffset)
    {
    }

    Impl(
            std::int16_t year, std::int8_t month, std::int8_t day,
            std::int8_t hour, std::int8_t minute, std::int8_t second,
            std::int16_t tzOffsetMinutes)
        : dateTime_(
              boost::gregorian::date(year, month, day),
              boost::posix_time::hours(hour) +
              boost::posix_time::minutes(minute) +
              boost::posix_time::seconds(second)),
          tzOffset_(tzOffsetMinutes)
    {
        if (dateTime_.is_special())
        {
            throw InvalidDateTime("Not a valid RFC 3339 timestamp: "
                                  "date/time out of range");
        }
        if ((tzOffset_ <= boost::posix_time::hours(-24)) ||
            (tzOffset_ > boost::posix_time::hours(24)))
        {
            throw InvalidDateTime("Not a valid RFC 3339 timestamp: "
                                  "timezone out of range");
        }
    }

    boost::posix_time::ptime utc() const
    {
        kulloAssert(!dateTime_.is_special());
        return dateTime_ - tzOffset_;
    }

    boost::posix_time::time_duration operator-(const Impl &rhs) const
    {
        return this->utc() - rhs.utc();
    }

    bool operator==(const Impl &other) const
    {
        if (this->dateTime_.is_special() || other.dateTime_.is_special())
        {
            return this->dateTime_ == other.dateTime_;
        }
        return this->utc() == other.utc();
    }

    bool operator<(const Impl &other) const
    {
        return this->utc() < other.utc();
    }

    bool operator>(const Impl &other) const
    {
        return this->utc() > other.utc();
    }
};

DateTime::DateTime()
    : impl_(make_unique<Impl>())
{
}

DateTime::DateTime(const DateTime &other)
    : impl_(make_unique<Impl>(*(other.impl_)))
{
}

DateTime &DateTime::operator=(DateTime other)
{
    std::swap(impl_, other.impl_);
    return *this;
}

DateTime::DateTime(const std::string &str)
    : DateTime()
{
    if (str.empty()) throw EmptyDateTime();

    boost::cmatch matches;
    if (boost::regex_match(str.c_str(), matches, RFC3339_REGEX))
    {
        size_t pos = 1;  // skip full match
        int year = std::atoi(matches[pos++].first);
        int month = std::atoi(matches[pos++].first);
        int day = std::atoi(matches[pos++].first);
        int hour = std::atoi(matches[pos++].first);
        int minute = std::atoi(matches[pos++].first);
        int second = std::atoi(matches[pos++].first);

        int tzOffsetMinutes = 0;
        const auto &matchSign = matches[pos++];
        if (matchSign.matched)
        {
            int sign = (*(matchSign.first) == '+') ? +1 : -1;
            int tzHour = std::atoi(matches[pos++].first);
            int tzMinute = std::atoi(matches[pos++].first);
            tzOffsetMinutes = sign * (tzHour * 60 + tzMinute);
        }

        impl_.reset(new Impl(
                        Util::numeric_cast<std::int16_t>(year),
                        Util::numeric_cast<std::int8_t>(month),
                        Util::numeric_cast<std::int8_t>(day),
                        Util::numeric_cast<std::int8_t>(hour),
                        Util::numeric_cast<std::int8_t>(minute),
                        Util::numeric_cast<std::int8_t>(second),
                        Util::numeric_cast<std::int16_t>(tzOffsetMinutes)));
    }
    else
    {
        throw InvalidDateTime("Not a valid RFC 3339 timestamp: bad format");
    }
}

DateTime::DateTime(
        std::int16_t year, std::int8_t month, std::int8_t day,
        std::int8_t hour, std::int8_t minute, std::int8_t second,
        int16_t tzOffsetMinutes)
    : impl_(new Impl(year, month, day, hour, minute, second, tzOffsetMinutes))
{
}

DateTime::~DateTime()
{
}

DateTime DateTime::epoch()
{
    return DateTime(1970, 1, 1, 0, 0, 0, 0);
}

DateTime DateTime::nowUtc()
{
    DateTime result(
                make_unique<Impl>(
                    boost::posix_time::second_clock::universal_time(),
                    boost::posix_time::minutes(0)));
    return result;
}

std::string DateTime::toString() const
{
    auto date = impl_->dateTime_.date();
    auto time = impl_->dateTime_.time_of_day();
    auto tzOffsetSeconds = impl_->tzOffset_.total_seconds();
    char sign = (tzOffsetSeconds >= 0) ? '+' : '-';
    tzOffsetSeconds = std::abs(tzOffsetSeconds);
    auto tzHours = (tzOffsetSeconds / 60) / 60;
    auto tzMinutes = (tzOffsetSeconds / 60) % 60;

    auto formatted = boost::format("%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d")
            % date.year() % date.month() % date.day()
            % time.hours() % time.minutes() % time.seconds()
            % sign % tzHours % tzMinutes;
    return formatted.str();
}

std::int16_t DateTime::year() const
{
    // Need to manually convert boost::gregorian::greg_year to a number due to
    // some ambiguity in template instantiation.
    // boost::gregorian::greg_year has no as_number() member...
    return Util::numeric_cast<std::int16_t>(impl_->dateTime_.date().year().operator unsigned short());
}

std::int8_t DateTime::month() const
{
    return Util::numeric_cast<std::int8_t>(impl_->dateTime_.date().month().as_number());
}

std::int8_t DateTime::day() const
{
    return Util::numeric_cast<std::int8_t>(impl_->dateTime_.date().day().as_number());
}

std::int8_t DateTime::hour() const
{
    return Util::numeric_cast<std::int8_t>(impl_->dateTime_.time_of_day().hours());
}

std::int8_t DateTime::minute() const
{
    return Util::numeric_cast<std::int8_t>(impl_->dateTime_.time_of_day().minutes());
}

std::int8_t DateTime::second() const
{
    return Util::numeric_cast<std::int8_t>(impl_->dateTime_.time_of_day().seconds());
}

std::int16_t DateTime::tzOffsetMinutes() const
{
    return Util::numeric_cast<std::int16_t>(impl_->tzOffset_.total_seconds() / 60);
}

bool DateTime::operator==(const DateTime &other) const
{
    return *impl_ == *(other.impl_);
}

bool DateTime::operator!=(const DateTime &other) const
{
    return !(*this == other);
}

bool DateTime::operator<(const DateTime &other) const
{
    return *impl_ < *(other.impl_);
}

bool DateTime::operator>(const DateTime &other) const
{
    return *impl_ > *(other.impl_);
}

bool DateTime::operator<=(const DateTime &other) const
{
    return (*this == other) || (*this < other);
}

bool DateTime::operator>=(const DateTime &other) const
{
    return (*this == other) || (*this > other);
}

DateTime operator+(const DateTime &lhs, std::chrono::seconds rhs)
{
    DateTime result(
                make_unique<DateTime::Impl>(
                    lhs.impl_->dateTime_
                    + boost::posix_time::seconds(rhs.count()),
                    lhs.impl_->tzOffset_));
    return result;
}

DateTime operator+(std::chrono::seconds lhs, const DateTime &rhs)
{
    return rhs + lhs;
}

DateTime DateTime::operator-(std::chrono::seconds delta) const
{
    return *this + -delta;
}

std::chrono::seconds DateTime::operator-(const DateTime &rhs) const
{
    auto boostDelta = this->impl_->utc() - rhs.impl_->utc();
    return std::chrono::seconds(boostDelta.total_seconds());
}

DateTime::DateTime(std::unique_ptr<DateTime::Impl> impl)
    : impl_(std::move(impl))
{
}

std::ostream &operator<<(std::ostream &out, const DateTime &dateTime)
{
    return out << dateTime.toString();
}

}
}
