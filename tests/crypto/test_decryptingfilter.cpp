/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/crypto/decryptingfilter.h>
#include <kulloclient/crypto/symmetrickeygenerator.h>
#include <kulloclient/crypto/symmetrickeyloader.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/hex.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

namespace {
const std::vector<unsigned char> PLAINTEXT = Util::to_vector("Hello, world!");
const std::vector<unsigned char> KEY = Util::Hex::decode(
            "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
// using AES-256/GCM with a tag size of 16 bytes
const std::vector<unsigned char> CIPHERTEXT = Util::Hex::decode(
            "428687d56d63ee0c362feee1e35ac8f5d30f3d922661edc179d0c2b7f9");
}

class DecryptingFilter : public KulloTest {};

K_TEST_F(DecryptingFilter, decryptsEmptyCiphertextToEmptyPlaintext)
{
    Crypto::SymmetricKey key =
            Crypto::SymmetricKeyGenerator().makeMessageSymmKey();

    std::vector<unsigned char> output;
    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Crypto::DecryptingFilter>(key));

    chain.close();
    EXPECT_THAT(output, IsEmpty());
}

K_TEST_F(DecryptingFilter, throwsOnInvalidData)
{
    Crypto::SymmetricKey key =
            Crypto::SymmetricKeyGenerator().makeMessageSymmKey();

    Util::FilterChain chain(make_unique<Util::NullSink>());
    chain.pushFilter(make_unique<Crypto::DecryptingFilter>(key));

    std::vector<unsigned char> input = {23, 42, 5};
    chain.write(input);
    EXPECT_ANY_THROW(chain.close());
}

K_TEST_F(DecryptingFilter, decryptsData)
{
    auto key = Crypto::SymmetricKeyLoader::fromVector(KEY);
    std::vector<unsigned char> output;

    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Crypto::DecryptingFilter>(key));

    chain.write(CIPHERTEXT);
    chain.close();
    EXPECT_THAT(output, Eq(PLAINTEXT));
}
