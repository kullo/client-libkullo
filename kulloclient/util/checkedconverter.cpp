/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/checkedconverter.h"

#include <limits>
#include <boost/regex.hpp>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/util/base64.h"

namespace Kullo {
namespace Util {

namespace {
const boost::regex COLOR_REGEX("\\A#[0-9a-fA-F]{6}\\z");
const boost::regex HEX_REGEX("\\A[0-9a-fA-F]*\\z");
const double LARGEST_UINT64_IN_DOUBLE = 1ULL<<53; //2^53, 52bits of mantissa + the implicit leading 1
}

// IMPORTANT: Whenever converting from Json::Value, check isXYZ() before using toXYZ()!
// toXYZ() is a bit too generous, converting null to false or 0, for example.

bool CheckedConverter::toBool(const Json::Value &value)
{
    if (!value.isBool())
    {
        throw ConversionException("couldn't convert value to bool");
    }
    return value.asBool();
}

bool CheckedConverter::toBool(const Json::Value &value, bool defaultVal)
{
    if (value.isNull()) return defaultVal;
    return toBool(value);
}

std::uint32_t CheckedConverter::toUint32(const Json::Value &value)
{
    if (!value.isUInt())
    {
        throw ConversionException("couldn't convert non-uint value to uint32");
    }
    try
    {
        return value.asUInt();
    }
    catch (...)
    {
        std::throw_with_nested(ConversionException("couldn't convert value to uint32"));
        return 0;  // make compiler happy
    }
}

std::uint32_t CheckedConverter::toUint32(const Json::Value &value, std::uint32_t defaultVal)
{
    if (value.isNull()) return defaultVal;
    return toUint32(value);
}

std::uint64_t CheckedConverter::toUint64(const Json::Value &value)
{
    if (!value.isUInt64())
    {
        throw ConversionException("couldn't convert non-uint value to uint64");
    }
    if (value.asDouble() > LARGEST_UINT64_IN_DOUBLE)
    {
        throw ConversionException("couldn't losslessly convert value greater than 2^53 to uint64");
    }
    try
    {
        return value.asUInt64();
    }
    catch (...)
    {
        std::throw_with_nested(ConversionException("couldn't convert value to uint64"));
        return 0;  // make compiler happy
    }
}

std::uint64_t CheckedConverter::toUint64(const Json::Value &value, std::uint64_t defaultVal)
{
    if (value.isNull()) return defaultVal;
    return toUint64(value);
}

std::string CheckedConverter::toString(const Json::Value &value, AllowEmpty allowEmpty)
{
    if (!value.isString())
    {
        throw ConversionException("couldn't convert value to string");
    }
    std::string string = value.asString();
    if (allowEmpty == AllowEmpty::False && string.empty())
    {
        throw ConversionException("couldn't convert value to non-empty string");
    }
    return string;
}

std::string CheckedConverter::toString(const Json::Value &value, const std::string &defaultVal)
{
    if (value.isNull()) return defaultVal;

    AllowEmpty allowEmpty = defaultVal.empty() ? AllowEmpty::True : AllowEmpty::False;
    return toString(value, allowEmpty);
}

bool CheckedConverter::isValidHexString(const std::string &value)
{
    return boost::regex_match(value, HEX_REGEX);
}

std::string CheckedConverter::toHexString(const Json::Value &value, AllowEmpty allowEmpty)
{
    std::string result = toString(value, allowEmpty);
    if (!isValidHexString(result))
    {
        throw ConversionException("couldn't convert value to hex string (not a valid hex value)");
    }
    return result;
}

std::string CheckedConverter::toHexString(const Json::Value &value, const std::string &defaultVal)
{
    if (value.isNull()) return defaultVal;

    AllowEmpty allowEmpty = defaultVal.empty() ? AllowEmpty::True : AllowEmpty::False;
    return toHexString(value, allowEmpty);
}

bool CheckedConverter::isValidColorString(const std::string &value)
{
    if (value.empty()) return true;
    return boost::regex_match(value, COLOR_REGEX);
}

std::string CheckedConverter::toColorString(const Json::Value &value, AllowEmpty allowEmpty)
{
    std::string result = toString(value, allowEmpty);
    if (!isValidColorString(result))
    {
        throw ConversionException("couldn't convert value to color string (not a valid color)");
    }
    return result;
}

std::string CheckedConverter::toColorString(const Json::Value &value, const std::string &defaultVal)
{
    if (value.isNull()) return defaultVal;

    AllowEmpty allowEmpty = defaultVal.empty() ? AllowEmpty::True : AllowEmpty::False;
    return toColorString(value, allowEmpty);
}

DateTime CheckedConverter::toDateTime(const std::string &value)
{
    if (value.empty()) return DateTime();

    try
    {
        return DateTime(value);
    }
    catch (...)
    {
        std::throw_with_nested(ConversionException("couldn't convert value to DateTime (not a valid date/time)"));
        return DateTime();  // silence "missing return" warning
    }
}

DateTime CheckedConverter::toDateTime(const Json::Value &value, AllowEmpty allowEmpty)
{
    std::string string = toString(value, allowEmpty);
    return toDateTime(string);
}

DateTime CheckedConverter::toDateTime(const Json::Value &value, const DateTime &defaultVal)
{
    if (value.isNull()) return defaultVal;

    AllowEmpty allowEmpty = defaultVal.isNull() ? AllowEmpty::True : AllowEmpty::False;
    return toDateTime(value, allowEmpty);
}

KulloAddress CheckedConverter::toKulloAddress(const Json::Value &value)
{
    auto address_string = toString(value, AllowEmpty::False); // might throw ConversionException

    try {
        return KulloAddress(address_string);
    }
    catch (std::invalid_argument)
    {
        std::throw_with_nested(ConversionException("string is not a valid KulloAddress"));
    }
}

std::vector<unsigned char> CheckedConverter::toVector(const Json::Value &value, AllowEmpty allowEmpty)
{
    std::string stringEncoded = toString(value, allowEmpty);
    auto out = std::vector<unsigned char>();
    try
    {
        out = Base64::decode(stringEncoded);
    }
    catch (...)
    {
        std::throw_with_nested(ConversionException("Couldn't decode value from base64."));
    }
    return out;
}

std::vector<unsigned char> CheckedConverter::toVector(const Json::Value &value, const std::vector<unsigned char> &defaultVal)
{
    if (value.isNull()) return defaultVal;

    AllowEmpty allowEmpty = defaultVal.empty() ? AllowEmpty::True : AllowEmpty::False;
    return toVector(value, allowEmpty);
}

}
}
