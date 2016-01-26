/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/urlencoding.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

K_TEST(UrlEncoding, emptyEncodesToEmpty)
{
    EXPECT_THAT(Util::UrlEncoding::encode(""), Eq(""));
}

K_TEST(UrlEncoding, preservesUnreservedChars)
{
    const std::string data = "abcdefghijklmnopqrstuvwxyz0123456789-._~";
    EXPECT_THAT(Util::UrlEncoding::encode(data), Eq(data));
}

K_TEST(UrlEncoding, encodesReservedChars)
{
    const std::string data = ":/?#[]@!$&'()*+,;=%";
    const std::string expected =
            "%3A%2F%3F%23%5B%5D%40%21%24%26%27%28%29%2A%2B%2C%3B%3D%25";
    EXPECT_THAT(Util::UrlEncoding::encode(data), Eq(expected));
}
