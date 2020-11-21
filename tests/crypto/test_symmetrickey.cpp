/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/crypto/symmetrickeygenerator.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class SymmetricKey : public KulloTest
{
};

K_TEST_F(SymmetricKey, masterKeyHasRightLength)
{
    Crypto::SymmetricKeyGenerator keyGen;

    auto desiredBitSize = Crypto::SymmetricKeyGenerator::MASTER_KEY_BITS;
    EXPECT_THAT(keyGen.makeMasterKey().bitSize(), Eq(desiredBitSize));
}

K_TEST_F(SymmetricKey, loginKeyHasRightLength)
{
    Crypto::SymmetricKeyGenerator keyGen;
    Util::MasterKey masterKey = Util::MasterKey(keyGen.makeMasterKey().toVector());
    Util::KulloAddress user("tester#kullo.net");

    auto desiredBitSize = Crypto::SymmetricKeyGenerator::LOGIN_KEY_BITS;
    EXPECT_THAT(keyGen.makeLoginKey(user, masterKey).bitSize(), Eq(desiredBitSize));
}

K_TEST_F(SymmetricKey, privateDataKeyHasRightLength)
{
    Crypto::SymmetricKeyGenerator keyGen;

    auto desiredBitSize = Crypto::SymmetricKeyGenerator::PRIVATE_DATA_KEY_BITS;
    EXPECT_THAT(keyGen.makePrivateDataKey().bitSize(), Eq(desiredBitSize));
}
