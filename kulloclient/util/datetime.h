/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
    DateTime();
    DateTime(const DateTime &other);
    DateTime &operator=(DateTime other);
    explicit DateTime(const std::string &str);
    DateTime(int year, int month, int day, int hour, int minute, int second, int tzOffset = 0);
    virtual ~DateTime();
    static DateTime epoch();
    static DateTime nowUtc();

    /// Returns the date in rfc3339 format with full seconds and numeric timezone,
    /// e.g. 1937-01-01T12:00:27+02:00
    std::string toString() const;
    bool isNull() const;

    int year() const;
    int month() const;
    int day() const;
    int hour() const;
    int minute() const;
    int second() const;
    int tzOffset() const;

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

    std::unique_ptr<Impl> impl_;
};

}
}
