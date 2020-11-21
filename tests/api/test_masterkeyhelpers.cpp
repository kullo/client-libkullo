/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/api/MasterKeyHelpers.h>
#include <kulloclient/api_impl/MasterKey.h>

#include "tests/kullotest.h"
#include "tests/testdata.h"

using namespace testing;
using namespace Kullo;
using namespace MasterKeyData;

K_TEST(ApiMasterKeyStatic, toPemWorks)
{
    auto masterKey = Api::MasterKey(VALID_DATA_BLOCKS);
    EXPECT_THAT(Api::MasterKeyHelpers::toPem(masterKey), Eq(VALID_PEM));
}

K_TEST(ApiMasterKeyStatic, createFromPemFailsForInvalidKey)
{
    EXPECT_THAT(Api::MasterKeyHelpers::createFromPem(INVALID_PEM),
                Eq(boost::none));
}

K_TEST(ApiMasterKeyStatic, createFromPemWorks)
{
    EXPECT_THAT(Api::MasterKeyHelpers::createFromPem(VALID_PEM),
                Ne(boost::none));
}

K_TEST(ApiMasterKeyStatic, createFromDataBlocksFailsForInvalidKey)
{
    EXPECT_THAT(Api::MasterKeyHelpers::createFromDataBlocks(INVALID_DATA_BLOCKS),
                Eq(boost::none));
}

K_TEST(ApiMasterKeyStatic, createFromDataBlocksWorks)
{
    EXPECT_THAT(Api::MasterKeyHelpers::createFromDataBlocks(VALID_DATA_BLOCKS),
                Ne(boost::none));
}

K_TEST(ApiMasterKeyStatic, isValidBlockWorks)
{
    // wrong length
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock(""), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("0"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("0000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("0000000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000000000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("0000000000"), Eq(false));

    // wrong chars (end, middle, begin)
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000a"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000E"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000/"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000?"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000 "), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000\t"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("00000\n"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000a00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000E00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000/00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000?00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000 00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000\t00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000\n00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("a00"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("E00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("/00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("?00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock(" 00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("\t00000"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("\n00000"), Eq(false));

    // correct chars
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("000000"), Eq(true));

    // Correct block modified at the end
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170530"), Eq(true));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170531"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170532"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170533"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170534"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170535"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170536"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170537"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170538"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170539"), Eq(false));

    // Correct block modified at the beginning
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("170530"), Eq(true));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("070530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("270530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("370530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("470530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("570530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("670530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("770530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("870530"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("970530"), Eq(false));

    // Range (16 bit ~ [0, 65535])
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("655357"), Eq(true));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("655365"), Eq(false));
    EXPECT_THAT(Api::MasterKeyHelpers::isValidBlock("999995"), Eq(false));
}

K_TEST(ApiMasterKeyStatic, encryptAndDecryptWork)
{
    auto password = "hello world";
    auto masterKey = Api::MasterKey(VALID_DATA_BLOCKS);

    auto encrypted = Api::MasterKeyHelpers::encrypt(password, masterKey);

    EXPECT_THAT(Api::MasterKeyHelpers::decrypt(password, encrypted), Eq(masterKey));
    EXPECT_THAT(Api::MasterKeyHelpers::decrypt("wrong", encrypted), Eq(boost::none));
}
