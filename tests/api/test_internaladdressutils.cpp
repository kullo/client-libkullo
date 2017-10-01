/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"

#include <kulloclient/api/InternalAddressUtils.h>

using namespace testing;
using namespace Kullo::Api;

K_TEST(ApiInternalAddressUtils, isValidWorks)
{
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "example.com"), Eq(true));
}

K_TEST(ApiInternalAddressUtils, isValidFailsOnInvalidData)
{
    // not an address
    EXPECT_THAT(InternalAddressUtils::isValid("", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", ""), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "123"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid(" ab", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab ", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", " example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "example.com "), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("^ab", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab^", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "^example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "example.com^"), Eq(false));

    // not normalized
    EXPECT_THAT(InternalAddressUtils::isValid("Ab", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("aB", "example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "Example.com"), Eq(false));
    EXPECT_THAT(InternalAddressUtils::isValid("ab", "examPle.com"), Eq(false));
}
