/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/crypto/symmetrickey.h>
#include <kulloclient/crypto/symmetrickeyloader.h>
#include <kulloclient/util/masterkey.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

namespace {
    struct TestData
    {
        const Util::MasterKey keyAsMasterKey = Util::MasterKey(
                    "-----BEGIN KULLO PRIVATE MASTER KEY-----\n"
                    "Version: 1 (256 bit)\n"
                    "\n"
                    "266940 297838 284810 310839\n"
                    "282350 203356 292573 169847\n"
                    "218669 264473 287888 287581\n"
                    "305318 262212 202945 218388\n"
                    "-----END KULLO PRIVATE MASTER KEY-----\n"
                    );
        const std::vector<unsigned char> keyAsVector = std::vector<unsigned char>{
            'h', 'F', 't', 'W', 'o', 'A', 'y', 'k',
            'n', 'K', 'O', 'o', 'r', 'I', 'B', 'X',
            'U', 'j', 'g', 'O', 'p', 't', 'p', 'V',
            'w', 'C', 'f', 'm', 'O', 'F', 'U', 'N'
        };

        const std::vector<unsigned char> vector0bytes = std::vector<unsigned char>{ };
        const std::vector<unsigned char> vector1bytes = std::vector<unsigned char>{ 'A' };
        const std::vector<unsigned char> vector2bytes = std::vector<unsigned char>{ 'A', 'B' };
        const std::vector<unsigned char> vector4bytes = std::vector<unsigned char>{ 'A', 'B', 'X', 'Z' };
        const std::vector<unsigned char> vector8bytes = std::vector<unsigned char>{ 'A', 'B', 'X', 'Z', 'a', 'b', 'x', 'z' };
        const std::string vector0bytesHex = "";
        const std::string vector1bytesHex = "41";
        const std::string vector2bytesHex = "4142";
        const std::string vector4bytesHex = "4142585a";
        const std::string vector8bytesHex = "4142585a6162787a";
    };
}

class SymmetricKeyLoader : public KulloTest
{
protected:
    Crypto::SymmetricKeyLoader loader;
};

K_TEST_F(SymmetricKeyLoader, fromMasterKeyReturnsCorrectLength)
{
    TestData data;

    auto output = loader.fromMasterKey(data.keyAsMasterKey);
    EXPECT_THAT(output.bitSize(), Eq(size_t{256}));
}

K_TEST_F(SymmetricKeyLoader, fromMasterKeyReturnsCorrectData)
{
    TestData data;

    auto output = loader.fromMasterKey(data.keyAsMasterKey);
    EXPECT_THAT(output, Eq(loader.fromVector(data.keyAsVector)));
}

K_TEST_F(SymmetricKeyLoader, fromVectorReturnsCorrectLength)
{
    TestData data;

    auto output0 = loader.fromVector(data.vector0bytes);
    auto output1 = loader.fromVector(data.vector1bytes);
    auto output2 = loader.fromVector(data.vector2bytes);
    auto output4 = loader.fromVector(data.vector4bytes);
    auto output8 = loader.fromVector(data.vector8bytes);

    EXPECT_THAT(output0.bitSize(), Eq(size_t{0*8}));
    EXPECT_THAT(output1.bitSize(), Eq(size_t{1*8}));
    EXPECT_THAT(output2.bitSize(), Eq(size_t{2*8}));
    EXPECT_THAT(output4.bitSize(), Eq(size_t{4*8}));
    EXPECT_THAT(output8.bitSize(), Eq(size_t{8*8}));
}

K_TEST_F(SymmetricKeyLoader, fromVectorReturnsCorrectData)
{
    TestData data;

    auto output0 = loader.fromVector(data.vector0bytes);
    auto output1 = loader.fromVector(data.vector1bytes);
    auto output2 = loader.fromVector(data.vector2bytes);
    auto output4 = loader.fromVector(data.vector4bytes);
    auto output8 = loader.fromVector(data.vector8bytes);

    auto expected0 = Crypto::SymmetricKey(data.vector0bytesHex);
    auto expected1 = Crypto::SymmetricKey(data.vector1bytesHex);
    auto expected2 = Crypto::SymmetricKey(data.vector2bytesHex);
    auto expected4 = Crypto::SymmetricKey(data.vector4bytesHex);
    auto expected8 = Crypto::SymmetricKey(data.vector8bytesHex);

    EXPECT_THAT(output0, Eq(expected0));
    EXPECT_THAT(output1, Eq(expected1));
    EXPECT_THAT(output2, Eq(expected2));
    EXPECT_THAT(output4, Eq(expected4));
    EXPECT_THAT(output8, Eq(expected8));
}
