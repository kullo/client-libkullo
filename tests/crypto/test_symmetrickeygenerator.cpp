/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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

K_TEST_F(SymmetricKeyGenerator, makeStorageKeyWorks)
{
    auto inputMasterKey = Util::MasterKey(
                Util::to_vector("hFtWoAyknKOorIBXUjgOptpVwCfmOFUN"));
    auto expectedKey = Crypto::SymmetricKey(
                "1732ab5e72af1434a26031af32cdbb24"
                "cc42842de09035732196eddefef40ff8"
                "f9cedb765ecb8889b617594d3d1b10af"
                "ac40fd769a8193eaee40433ff474b00d"
                "0c14e57ae61c2d9d6fabeb9127f35b2c"
                "f4affcc1d01c7e5008254693771d23ca");

    Crypto::SymmetricKeyGenerator keyGen;
    EXPECT_THAT(keyGen.makeStorageKey(
                    Util::KulloAddress("hi#kullo.net"), inputMasterKey),
                Eq(expectedKey));
}

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
    auto expectedKey = Crypto::SymmetricKey(
                "481acd52464821ddadeff998e1be8892"
                "2fe8ce8cd8795172115d6f8852997f4c"
                "efe721ee801ece84e0297c6045ee41ee"
                "5ac2748a76e34edb9ff99c4157d8528e");

    Crypto::SymmetricKeyGenerator keyGen;
    EXPECT_THAT(keyGen.makeLoginKey(
                    Util::KulloAddress("hi#kullo.net"), inputMasterKey),
                Eq(expectedKey));
}

K_TEST_F(SymmetricKeyGenerator, makeLoginKeyDerivesTheRightKeyFromBinary)
{
    auto inputMasterKey = Util::MasterKey(
                Util::to_vector("hFtWoAyknKOorIBXUjgOptpVwCfmOFUN"));
    auto expectedKey = Crypto::SymmetricKey(
                "481acd52464821ddadeff998e1be8892"
                "2fe8ce8cd8795172115d6f8852997f4c"
                "efe721ee801ece84e0297c6045ee41ee"
                "5ac2748a76e34edb9ff99c4157d8528e");

    Crypto::SymmetricKeyGenerator keyGen;
    EXPECT_THAT(keyGen.makeLoginKey(
                    Util::KulloAddress("hi#kullo.net"),
                    inputMasterKey),
                Eq(expectedKey));
}
