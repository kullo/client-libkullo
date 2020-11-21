/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/apitest.h"

#include <kulloclient/api/InternalDateTimeUtils.h>
#include <kulloclient/api_impl/DateTime.h>
#include <kulloclient/util/exceptions.h>

using namespace testing;
using namespace Kullo;

K_TEST(ApiInternalDateTimeUtils, isValidFailsOnInvalidData)
{
    EXPECT_THAT(Api::InternalDateTimeUtils::isValid(2015, 13, 1, 0, 0, 0, 0),
                Eq(false));
}

K_TEST(ApiInternalDateTimeUtils, isValidWorks)
{
    EXPECT_THAT(Api::InternalDateTimeUtils::isValid(2015, 1, 1, 0, 0, 0, 0),
                Eq(true));
}

K_TEST(ApiInternalDateTimeUtils, nowUtcWorks)
{
    EXPECT_NO_THROW(Api::InternalDateTimeUtils::nowUtc());
}

K_TEST(ApiInternalDateTimeUtils, compareWorks)
{
    auto earlier = Api::DateTime(2015, 1, 1, 0, 0, 0, 0);
    auto later   = Api::DateTime(2015, 1, 1, 0, 0, 1, 0);
    EXPECT_THAT(Api::InternalDateTimeUtils::compare(earlier, later), Eq(-1));
    EXPECT_THAT(Api::InternalDateTimeUtils::compare(later, earlier), Eq( 1));
    EXPECT_THAT(Api::InternalDateTimeUtils::compare(earlier, earlier), Eq(0));
}
