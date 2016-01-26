/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/crypto/symmetrickey.h>
#include <kulloclient/crypto/symmetrickeygenerator.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/kulloaddress.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class SymmetricKeyGenerator : public KulloTest
{
};

K_TEST_F(SymmetricKeyGenerator, makeLoginKeyDerivesTheRightKeyFromBlockList)
{
    auto input = std::string(
                "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                "Version: 1 (256 bit)\n"
                "\n"
                "266940 297838 284810 310839\n"
                "282350 203356 292573 169847\n"
                "218669 264473 287888 287581\n"
                "305318 262212 202945 218388\n"
                "-----END KULLO PRIVATE MASTER KEY-----\n"
                );
    auto inputMasterKey = Util::MasterKey(input);
    auto expectedKey = Crypto::SymmetricKey("481acd52464821ddadeff998e1be88922fe8ce8cd8795172115d6f8852997f4cefe721ee801ece84e0297c6045ee41ee5ac2748a76e34edb9ff99c4157d8528e");

    Crypto::SymmetricKeyGenerator keyGen;
    EXPECT_THAT(keyGen.makeLoginKey(Util::KulloAddress("hi#kullo.net"), inputMasterKey), Eq(expectedKey));
}

K_TEST_F(SymmetricKeyGenerator, makeLoginKeyDerivesTheRightKeyFromBinary)
{
    auto inputMasterKey = Util::MasterKey(Util::to_vector("hFtWoAyknKOorIBXUjgOptpVwCfmOFUN"));
    auto expectedKey = Crypto::SymmetricKey("481acd52464821ddadeff998e1be88922fe8ce8cd8795172115d6f8852997f4cefe721ee801ece84e0297c6045ee41ee5ac2748a76e34edb9ff99c4157d8528e");

    Crypto::SymmetricKeyGenerator keyGen;
    EXPECT_THAT(keyGen.makeLoginKey(Util::KulloAddress("hi#kullo.net"), inputMasterKey), Eq(expectedKey));
}
