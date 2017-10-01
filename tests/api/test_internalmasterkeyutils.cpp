/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/api/apitest.h"
#include "tests/testdata.h"

#include <kulloclient/api/InternalMasterKeyUtils.h>

using namespace testing;
using namespace Kullo::Api;

K_TEST(ApiInternalMasterKeyUtils, isValidWorks)
{
    EXPECT_THAT(InternalMasterKeyUtils::isValid(MasterKeyData::VALID_DATA_BLOCKS), Eq(true));
}

K_TEST(ApiInternalMasterKeyUtils, isValidFailsOnInvalidData)
{
    EXPECT_THAT(InternalMasterKeyUtils::isValid(MasterKeyData::INVALID_DATA_BLOCKS), Eq(false));
}
