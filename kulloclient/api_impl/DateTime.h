/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/DateTime_base.h"

#include <chrono>
#include <iosfwd>
#include <boost/operators.hpp>
#include <boost/optional/optional_fwd.hpp>

#include "kulloclient/util/datetime.h"

namespace Kullo {
namespace Api {

struct DateTime
        : public DateTimeBase
        , boost::equality_comparable<DateTime>
        , boost::less_than_comparable<DateTime>
{
    // Api::DateTimeBase
    DateTime(
            int16_t year, int8_t month, int8_t day,
            int8_t hour, int8_t minute, int8_t second,
            int16_t tzOffsetMinutes
            );
    static boost::optional<DateTime> fromRfc3339(const std::string &timeStr);

    explicit DateTime(const Util::DateTime &dateTime);
    std::string toString() const;

    bool operator==(const DateTime &rhs) const;
    bool operator<(const DateTime &rhs) const;
    friend std::ostream &operator<<(std::ostream &lhs, const DateTime &rhs);
    friend DateTime operator+(const DateTime &lhs, std::chrono::seconds rhs);
    friend DateTime operator+(std::chrono::seconds lhs, const DateTime &rhs);

    static DateTime nowUtc();

private:
    Util::DateTime dateTime_;
};

}
}
