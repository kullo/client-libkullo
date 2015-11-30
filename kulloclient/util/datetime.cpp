/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/datetime.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/misc.h"

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
    const boost::posix_time::seconds tzOffset_ = boost::posix_time::seconds(0);

    Impl() {}

    Impl(const boost::posix_time::ptime &dateTime, const boost::posix_time::seconds &tzOffset)
        : dateTime_(dateTime),
          tzOffset_(tzOffset)
    {
    }

    Impl(int year, int month, int day, int hour, int minute, int second, int tzOffset)
        : dateTime_(
              boost::gregorian::date(year, month, day),
              boost::posix_time::hours(hour) +
              boost::posix_time::minutes(minute) +
              boost::posix_time::seconds(second)),
          tzOffset_(tzOffset)
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

        int tzOffset = 0;
        const auto &matchSign = matches[pos++];
        if (matchSign.matched)
        {
            int sign = (*(matchSign.first) == '+') ? +1 : -1;
            int tzHour = std::atoi(matches[pos++].first);
            int tzMinute = std::atoi(matches[pos++].first);
            tzOffset = sign * (tzHour * 60 + tzMinute) * 60;
        }

        impl_.reset(new Impl(year, month, day, hour, minute, second, tzOffset));
    }
    else
    {
        throw InvalidDateTime("Not a valid RFC 3339 timestamp: bad format");
    }
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int tzOffset)
    : impl_(new Impl(year, month, day, hour, minute, second, tzOffset))
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
                    boost::posix_time::seconds(0)));
    return result;
}

std::string DateTime::toString() const
{
    kulloAssert(!isNull());

    auto date = impl_->dateTime_.date();
    auto time = impl_->dateTime_.time_of_day();
    auto tz = impl_->tzOffset_.total_seconds();
    char sign = (tz >= 0) ? '+' : '-';
    tz = std::abs(tz);
    auto tzHours = (tz / 60) / 60;
    auto tzMinutes = (tz / 60) % 60;

    auto formatted = boost::format("%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d")
            % date.year() % date.month() % date.day()
            % time.hours() % time.minutes() % time.seconds()
            % sign % tzHours % tzMinutes;
    return formatted.str();
}

bool DateTime::isNull() const
{
    return impl_->dateTime_.is_special();
}

int DateTime::year() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.date().year();
}

int DateTime::month() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.date().month();
}

int DateTime::day() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.date().day();
}

int DateTime::hour() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.time_of_day().hours();
}

int DateTime::minute() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.time_of_day().minutes();
}

int DateTime::second() const
{
    kulloAssert(!isNull());

    return impl_->dateTime_.time_of_day().seconds();
}

int DateTime::tzOffset() const
{
    kulloAssert(!isNull());

    return impl_->tzOffset_.total_seconds();
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
    kulloAssert(!lhs.isNull());

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
    if (dateTime.isNull()) return out << "(null)";

    return out << dateTime.toString();
}

}
}
