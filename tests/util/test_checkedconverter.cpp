/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <boost/optional.hpp>
#include <jsoncpp/jsoncpp.h>

#include <kulloclient/util/binary.h>
#include <kulloclient/util/checkedconverter.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class CheckedConverter : public KulloTest
{
};

K_TEST_F(CheckedConverter, toBoolJson)
{
    EXPECT_THROW(Util::CheckedConverter::toBool(Json::Value()), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toBool(Json::Value(42)), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(false)), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(true)), Eq(true));

    // with default
    EXPECT_THROW(Util::CheckedConverter::toBool(Json::Value(42), false), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(false), false), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(true), false), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(false), true), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(true), true), Eq(true));

    // with default and null
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(Json::nullValue), false), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::toBool(Json::Value(Json::nullValue), true), Eq(true));
}

K_TEST_F(CheckedConverter, toUint32Json)
{
    // valid cases
    EXPECT_THAT(Util::CheckedConverter::toUint32(Json::Value(0)), Eq(0U));
    EXPECT_THAT(Util::CheckedConverter::toUint32(Json::Value(42)), Eq(42U));
    EXPECT_THAT(Util::CheckedConverter::toUint32(Json::Value(4294967295.0)), Eq(4294967295ULL));

    // invalid cases
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value()), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(-0.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(-23.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(-23.9)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(-42)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(0.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(23.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(23.9)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(4294967296.0)), Util::ConversionException);

    // default
    EXPECT_THROW(Util::CheckedConverter::toUint32(Json::Value(false), 42), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toUint32(Json::Value(), 23), Eq(23U));
    EXPECT_THAT(Util::CheckedConverter::toUint32(Json::Value(42), 23), Eq(42U));
}

K_TEST_F(CheckedConverter, toUint64Json)
{
    // valid cases
    EXPECT_THAT(Util::CheckedConverter::toUint64(Json::Value(0)), Eq(0ULL));
    EXPECT_THAT(Util::CheckedConverter::toUint64(Json::Value(42)), Eq(42ULL));
    // 2^53
    EXPECT_THAT(Util::CheckedConverter::toUint64(Json::Value(9007199254740992.0)), Eq(9007199254740992ULL));

    // invalid cases
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value()), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(-0.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(-23.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(-23.9)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(-42)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(0.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(23.1)), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(23.9)), Util::ConversionException);
    // 2^53 + 2 (2^53 + 1 is not representable and would be rounded down to 2^53)
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(9007199254740994.0)), Util::ConversionException);
    // 2^64
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(18446744073709551616.0)), Util::ConversionException);

    // default
    EXPECT_THROW(Util::CheckedConverter::toUint64(Json::Value(false), 42), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toUint64(Json::Value(), 23), Eq(23ULL));
    EXPECT_THAT(Util::CheckedConverter::toUint64(Json::Value(42), 23), Eq(42ULL));
}

K_TEST_F(CheckedConverter, toStringJson)
{
    EXPECT_THROW(Util::CheckedConverter::toString(Json::Value()), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value("foobar")), StrEq("foobar"));

    // Util::AllowEmpty
    EXPECT_THROW(Util::CheckedConverter::toString(Json::Value("")), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value(""), Util::AllowEmpty::True), StrEq(""));

    // default: input null
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value(), ""), StrEq(""));
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value(), "foo"), StrEq("foo"));

    // default: input empty
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value(""), ""), StrEq(""));
    EXPECT_THROW(Util::CheckedConverter::toString(Json::Value(""), "foo"), Util::ConversionException);

    // default: input nonempty
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value("bar"), ""), StrEq("bar"));
    EXPECT_THAT(Util::CheckedConverter::toString(Json::Value("bar"), "foo"), StrEq("bar"));
}

K_TEST_F(CheckedConverter, isValidColorString)
{
    EXPECT_THAT(Util::CheckedConverter::isValidColorString(""), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#000000"), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#012345"), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#6789ab"), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#cdef01"), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#ffffff"), Eq(true));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#ABCDEF"), Eq(true));

    EXPECT_THAT(Util::CheckedConverter::isValidColorString(" "), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("123abc"), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString(" #123abc"), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#123abc "), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#123"), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#012efg"), Eq(false));
    EXPECT_THAT(Util::CheckedConverter::isValidColorString("#012EFG"), Eq(false));
}

K_TEST_F(CheckedConverter, toColorStringJson)
{
    EXPECT_THROW(Util::CheckedConverter::toColorString(Json::Value()), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toColorString(Json::Value("#abc")), Util::ConversionException);
    EXPECT_THAT(std::string("#beef01"), Util::CheckedConverter::toColorString(Json::Value("#beef01")));

    // Util::AllowEmpty
    EXPECT_THROW(Util::CheckedConverter::toColorString(Json::Value("")), Util::ConversionException);
    EXPECT_THAT(std::string(""), Util::CheckedConverter::toColorString(Json::Value(""), Util::AllowEmpty::True));

    // default: input null
    EXPECT_THAT(std::string(""), Util::CheckedConverter::toColorString(Json::Value(), ""));
    EXPECT_THAT(std::string("#123abc"), Util::CheckedConverter::toColorString(Json::Value(), "#123abc"));

    // default: input empty
    EXPECT_THAT(std::string(""), Util::CheckedConverter::toColorString(Json::Value(""), ""));
    EXPECT_THROW(Util::CheckedConverter::toColorString(Json::Value(""), "#123abc"),
                 Util::ConversionException);

    // default: input nonempty
    EXPECT_THAT(std::string("#6789ab"), Util::CheckedConverter::toColorString(Json::Value("#6789ab"), ""));
    EXPECT_THAT(std::string("#6789ab"), Util::CheckedConverter::toColorString(Json::Value("#6789ab"), "#123abc"));
}

K_TEST_F(CheckedConverter, toDateTimeJson)
{
    const Util::DateTime testData(2013, 12, 16, 14, 39, 13, 0);
    const std::string converted = "2013-12-16T14:39:13+00:00";
    EXPECT_THAT(testData.toString(), StrEq(converted));

    using UUT = Util::CheckedConverter;

    // various bad time formats
    EXPECT_THROW(UUT::toDateTime(Json::Value("#abc")), Util::ConversionException);
    EXPECT_THROW(UUT::toDateTime(Json::Value("2013-12-16")), Util::ConversionException);
    EXPECT_THROW(UUT::toDateTime(Json::Value("14:39:13")), Util::ConversionException);
    EXPECT_THROW(UUT::toDateTime(Json::Value("2013-12-16T14:39:13")), Util::ConversionException);

    // various good time formats
    EXPECT_THAT(UUT::toDateTime(Json::Value("2013-12-17T01:39:13+11:00")), Eq(testData));
    EXPECT_THAT(UUT::toDateTime(Json::Value("2013-12-16T13:09:13-01:30")), Eq(testData));
    EXPECT_THAT(UUT::toDateTime(Json::Value("2013-12-16T13:09:13Z")), Ne(testData));

    // input null
    EXPECT_THROW(UUT::toDateTime(Json::Value()), Util::ConversionException);
    EXPECT_THAT(UUT::toDateTime(Json::Value(), testData), Eq(testData));

    // input empty
    EXPECT_THROW(UUT::toDateTime(Json::Value("")), Util::ConversionException);
    EXPECT_THROW(UUT::toDateTime(Json::Value(""), testData), Util::ConversionException);

    // input valid and nonempty
    const Util::DateTime otherDate("2014-01-01T00:00:00Z");
    EXPECT_THAT(UUT::toDateTime(Json::Value(converted)), Eq(testData));
    EXPECT_THAT(UUT::toDateTime(Json::Value(converted), otherDate), Eq(testData));
}

K_TEST_F(CheckedConverter, toKulloAddressJson)
{
    const auto testAddress = Util::KulloAddress("test#kullo.test");
    const auto converted = std::string("test#kullo.test");
    ASSERT_THAT(testAddress.toString(), StrEq(converted));

    // bad input: not a string
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value()),     Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value(101)),  Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value(3.14)), Util::ConversionException);

    // bad input: string but invalid KulloAddress
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("")),              Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("a")),             Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("2013-12-16")),    Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("14:39:13")),      Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("#abc")),          Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("user#abc")),      Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toKulloAddress(Json::Value("___#kullo.test")), Util::ConversionException);

    // good input
    EXPECT_THAT(Util::CheckedConverter::toKulloAddress(Json::Value(converted)), Eq(testAddress));
    EXPECT_THAT(Util::CheckedConverter::toKulloAddress(Json::Value("test#kullo.test")), Eq(testAddress));
    EXPECT_THAT(Util::CheckedConverter::toKulloAddress(Json::Value("test2#kullo.test")), Ne(testAddress));
}

K_TEST_F(CheckedConverter, toVectorJson)
{
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value()), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy")), Eq(Util::to_vector("foobar")));

    // Allow whitespace
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value(" Zm9vYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9v YmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy ")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("\nZm9vYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9v\nYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy\n")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("\rZm9vYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9v\rYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy\r")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("\tZm9vYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9v\tYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy\t")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("\t\r\n Zm9vYmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9v\t\r\n YmFy")), Eq(Util::to_vector("foobar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("Zm9vYmFy\t\r\n ")), Eq(Util::to_vector("foobar")));

    // Util::AllowEmpty
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("")), Util::ConversionException);
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value(""), Util::AllowEmpty::True), IsEmpty());

    // default: input null
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value(), Util::to_vector("")), IsEmpty());
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value(), Util::to_vector("foo")), Eq(Util::to_vector("foo")));

    // default: input empty
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value(""), Util::to_vector("")), IsEmpty());
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value(""), Util::to_vector("foo")), Util::ConversionException);

    // default: input nonempty
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("YmFy"), Util::to_vector("")), Eq(Util::to_vector("bar")));
    EXPECT_THAT(Util::CheckedConverter::toVector(Json::Value("YmFy"), Util::to_vector("foo")), Eq(Util::to_vector("bar")));

    // default: input no valid base64
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("aabb?")), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("aabb*")), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("aabb~")), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("aabbö")), Util::ConversionException);
    EXPECT_THROW(Util::CheckedConverter::toVector(Json::Value("aabbö")), Util::ConversionException);
}
