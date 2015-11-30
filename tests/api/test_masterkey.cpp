/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <kulloclient/api/MasterKey.h>

#include "tests/kullotest.h"
#include "tests/testdata.h"

using namespace testing;
using namespace Kullo;
using namespace MasterKeyData;

K_TEST(ApiMasterKeyStatic, createFromPemFailsForInvalidKey)
{
    EXPECT_THAT(Api::MasterKey::createFromPem(INVALID_PEM), IsNull());
}

K_TEST(ApiMasterKeyStatic, createFromPemWorks)
{
    EXPECT_THAT(Api::MasterKey::createFromPem(VALID_PEM), Not(IsNull()));
}

K_TEST(ApiMasterKeyStatic, createFromDataBlocksFailsForInvalidKey)
{
    EXPECT_THAT(Api::MasterKey::createFromDataBlocks(INVALID_DATA_BLOCKS),
                IsNull());
}

K_TEST(ApiMasterKeyStatic, createFromDataBlocksWorks)
{
    EXPECT_THAT(Api::MasterKey::createFromDataBlocks(VALID_DATA_BLOCKS),
                Not(IsNull()));
}

K_TEST(ApiMasterKeyStatic, isValidBlockWorks)
{
    // wrong length
    EXPECT_THAT(Api::MasterKey::isValidBlock(""), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("0"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("0000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("0000000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000000000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("0000000000"), Eq(false));

    // wrong chars (end, middle, begin)
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000a"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000E"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000/"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000?"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000 "), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000\t"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("00000\n"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000a00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000E00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000/00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000?00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000 00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000\t00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("000\n00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("a00"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("E00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("/00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("?00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock(" 00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("\t00000"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("\n00000"), Eq(false));

    // correct chars
    EXPECT_THAT(Api::MasterKey::isValidBlock("000000"), Eq(true));

    // Correct block modified at the end
    EXPECT_THAT(Api::MasterKey::isValidBlock("170530"), Eq(true));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170531"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170532"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170533"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170534"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170535"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170536"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170537"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170538"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("170539"), Eq(false));

    // Correct block modified at the beginning
    EXPECT_THAT(Api::MasterKey::isValidBlock("170530"), Eq(true));
    EXPECT_THAT(Api::MasterKey::isValidBlock("070530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("270530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("370530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("470530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("570530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("670530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("770530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("870530"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("970530"), Eq(false));

    // Range (16 bit ~ [0, 65535])
    EXPECT_THAT(Api::MasterKey::isValidBlock("655357"), Eq(true));
    EXPECT_THAT(Api::MasterKey::isValidBlock("655365"), Eq(false));
    EXPECT_THAT(Api::MasterKey::isValidBlock("999995"), Eq(false));
}


class ApiMasterKey : public KulloTest
{
protected:
    ApiMasterKey()
        : uut(Api::MasterKey::createFromDataBlocks(VALID_DATA_BLOCKS))
    {}

    std::shared_ptr<Api::MasterKey> uut;
};

K_TEST_F(ApiMasterKey, pemWorks)
{
    EXPECT_THAT(uut->pem(), Eq(VALID_PEM));
}

K_TEST_F(ApiMasterKey, dataBlocksWorks)
{
    EXPECT_THAT(uut->dataBlocks(), Eq(VALID_DATA_BLOCKS));
}

K_TEST_F(ApiMasterKey, isEqualToWorks)
{
    auto uut2 =  Api::MasterKey::createFromDataBlocks(VALID_DATA_BLOCKS);
    EXPECT_THAT(uut->isEqualTo(uut2), Eq(true));

    auto uut3 =  Api::MasterKey::createFromDataBlocks(VALID_DATA_BLOCKS2);
    EXPECT_THAT(uut->isEqualTo(uut3), Eq(false));
}
