/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/api/AddressHelpers.h>
#include <kulloclient/api_impl/Address.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

K_TEST(AddressHelpers, createFailsForInvalidAddress)
{
    EXPECT_THAT(Api::AddressHelpers::create(""), Eq(boost::none));
    EXPECT_THAT(Api::AddressHelpers::create("invalid@address"), Eq(boost::none));
}

K_TEST(AddressHelpers, createWorks)
{
    EXPECT_THAT(Api::AddressHelpers::create("example#kullo.net"), Ne(boost::none));
}
