/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
