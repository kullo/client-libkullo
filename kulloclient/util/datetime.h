/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

namespace Kullo {
namespace Util {

/**
 * @brief The DateTime struct
 *
 * Format: yyyy-mm-ddThh:mm:ss[.f+](Z|(+|-)hh:mm) (case-insensitive)
 */
class DateTime {
public:
    DateTime(const DateTime &other);
    DateTime &operator=(DateTime other);
    explicit DateTime(const std::string &str);
    DateTime(
            std::int16_t year, std::int8_t month, std::int8_t day,
            std::int8_t hour, std::int8_t minute, std::int8_t second,
            std::int16_t tzOffsetMinutes = 0);
    virtual ~DateTime();
    static DateTime epoch();
    static DateTime nowUtc();

    /// Returns the date in rfc3339 format with full seconds and numeric timezone,
    /// e.g. 1937-01-01T12:00:27+02:00
    std::string toString() const;

    std::int16_t year() const;
    std::int8_t month() const;
    std::int8_t day() const;
    std::int8_t hour() const;
    std::int8_t minute() const;
    std::int8_t second() const;
    std::int16_t tzOffsetMinutes() const;

    bool operator==(const DateTime &other) const;
    bool operator!=(const DateTime &other) const;
    bool operator<(const DateTime &other) const;
    bool operator>(const DateTime &other) const;
    bool operator<=(const DateTime &other) const;
    bool operator>=(const DateTime &other) const;

    friend DateTime operator+(const DateTime &lhs, std::chrono::seconds rhs);
    friend DateTime operator+(std::chrono::seconds lhs, const DateTime &rhs);
    DateTime operator-(std::chrono::seconds delta) const;
    std::chrono::seconds operator-(const DateTime &rhs) const;

    friend std::ostream &operator<<(std::ostream &out, const DateTime &dateTime);

private:
    struct Impl;
    DateTime(std::unique_ptr<Impl> impl);
    DateTime();

    std::unique_ptr<Impl> impl_;
};

}
}
