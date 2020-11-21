/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <boost/optional/optional_fwd.hpp>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Util {

/**
 * @brief Thrown if the CheckedConverter cannot convert a value.
 */
class ConversionException : public BaseException
{
public:
    /// Creates a new ConversionException with the given message.
    ConversionException(const std::string &message) throw() : BaseException(message) {}
    ~ConversionException() = default;
};

/**
 * @brief Possible values for allow empty parameter in CheckedConverter functions
 */
enum struct AllowEmpty
{
    False, True
};

/// Converter for Json::Value to specific types.
class CheckedConverter
{
public:
    /**
     * @brief Converts the given Json::Value to bool.
     * @throw ConversionException if conversion fails
     *
     * The conversion fails if the Json::Value is not a bool.
     */
    static bool toBool(const Json::Value &value);

    /**
     * @brief Converts the given Json::Value to bool.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     */
    static bool toBool(const Json::Value &value, bool defaultVal);

    /**
     * @brief Converts the given Json::Value to uint32_t.
     * @throw ConversionException if conversion fails
     *
     * The conversion fails if the Json::Value is not a double,
     * if the numeric value is out of range of an uint32_t
     * or if it has a fractional part.
     */
    static std::uint32_t toUint32(const Json::Value &value);

    /**
     * @brief Converts the given Json::Value to uint32_t.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     */
    static std::uint32_t toUint32(const Json::Value &value, std::uint32_t defaultVal);

    /**
     * @brief Converts the given Json::Value to uint64.
     * @throw ConversionException if conversion fails
     *
     * The conversion fails if the Json::Value is not a double,
     * if the numeric value is out of range of an uint64
     * or if it has a fractional part.
     */
    static std::uint64_t toUint64(const Json::Value &value);

    /**
     * @brief Converts the given Json::Value to uint64.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     */
    static std::uint64_t toUint64(const Json::Value &value, std::uint64_t defaultVal);

    /**
     * @brief Converts the given Json::Value to string.
     * @throw ConversionException if conversion fails
     *
     * The conversion fails if the Json::Value is not a string, or
     * if the value is empty and allowEmpty is false.
     */
    static std::string toString(const Json::Value &value, AllowEmpty allowEmpty = AllowEmpty::False);

    /**
     * @brief Converts the given Json::Value to string.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     *
     * If the default value is not empty, the value is not allowed to be empty.
     */
    static std::string toString(const Json::Value &value, const std::string &defaultVal);

    /**
     * @brief Returns true iff the string consists of 0 or more hexadecimal chars.
     *
     * Both upper and lower case are accepted.
     */
    static bool isValidHexString(const std::string &value);

    /**
     * @brief Converts the given Json::Value to a string and verifies that it
     *        represents a hexadecimal value.
     * @throw ConversionException if conversion fails
     *
     * The conversion fails if the Json::Value is not a hex string, or
     * if the value is empty and allowEmpty is false.
     */
    static std::string toHexString(const Json::Value &value, AllowEmpty allowEmpty = AllowEmpty::False);

    /**
     * @brief Converts the given Json::Value to a string and verifies that it
     *        represents a hexadecimal value.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     *
     * If the default value is not empty, the value is not allowed to be empty.
     */
    static std::string toHexString(const Json::Value &value, const std::string &defaultVal);

    /// Returns true iff the string is empty or has a leading # and 6 hex places
    static bool isValidColorString(const std::string &value);

    /**
     * @brief Converts the given Json::Value to a color string (e.g. "#ffffff").
     * @param value Json::Value representing a string with leading # and 6 hex places
     * @param allowEmpty whether value may be an empty string
     * @throw ConversionException if conversion fails
     *
     * The conversion will fail if toString() fails when called with the same arguments,
     * or if the resulting string is no valid color string.
     */
    static std::string toColorString(const Json::Value &value, AllowEmpty allowEmpty = AllowEmpty::False);

    /**
     * @brief Converts the given Json::Value to a color string (e.g. "#ffffff").
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     *
     * If the default value is not empty, the value is not allowed to be empty.
     */
    static std::string toColorString(const Json::Value &value, const std::string &defaultVal);

    /**
     * @brief Converts the given string to DateTime.
     * @param value string of RFC3339 format
     * @throw ConversionException if conversion fails
     */
    static boost::optional<DateTime> toDateTime(const std::string &value);

    /**
     * @brief Converts the given Json::Value to DateTime.
     * @param value Json::Value representing a string of RFC3339 format
     * @param allowEmpty whether value may be an empty string
     * @throw ConversionException if conversion fails
     *
     * The conversion will fail if toString() fails when called with the same arguments,
     * or if the resulting string is not a valid ISO date/time string.
     */
    static DateTime toDateTime(const Json::Value &value);

    /**
     * @brief Converts the given Json::Value to DateTime.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     */
    static DateTime toDateTime(const Json::Value &value, const DateTime &defaultVal);

    /// Converts the given Json::Value to KulloAddress.
    static KulloAddress toKulloAddress(const Json::Value &value);

    /**
     * @brief Converts the given Json::Value to binary vector.
     * @param value Json::Value representing a string containing base64-encoded data
     * @param allowEmpty whether value may be an empty string
     * @throw ConversionException if conversion fails
     *
     * The conversion will fail if toString() fails when called with the same arguments
     * or the input is no valid base64 string.
     */
    static std::vector<unsigned char> toVector(const Json::Value &value, AllowEmpty allowEmpty = AllowEmpty::False);

    /**
     * @brief Converts the given Json::Value to binary vector.
     * @return defaultVal if value is undefined or null,
     *         otherwise same as the method without a default
     * @throw ConversionException if conversion fails
     *
     * If the default value is not empty, the value is not allowed to be empty.
     */
    static std::vector<unsigned char> toVector(const Json::Value &value, const std::vector<unsigned char> &defaultVal);
};

}
}
