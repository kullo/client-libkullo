/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/api_impl/MasterKey.h>

#include "tests/kullotest.h"
#include "tests/testdata.h"

using namespace testing;
using namespace Kullo;
using namespace MasterKeyData;

class ApiMasterKey : public KulloTest
{
protected:
    Api::MasterKey uut_ = VALID_DATA_BLOCKS;
};

K_TEST_F(ApiMasterKey, dataBlocksWorks)
{
    EXPECT_THAT(uut_.blocks, Eq(VALID_DATA_BLOCKS));
}

K_TEST_F(ApiMasterKey, isEqualToWorks)
{
    auto key2 = Api::MasterKey(VALID_DATA_BLOCKS);
    EXPECT_THAT(uut_, Eq(key2));

    auto key3 = Api::MasterKey(VALID_DATA_BLOCKS2);
    EXPECT_THAT(uut_, Ne(key3));
}
