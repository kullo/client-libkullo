/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/crypto/exceptions.h>
#include <kulloclient/crypto/hashverifyingfilter.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

namespace {

// echo -n | openssl sha512
const std::vector<char> SHA_512_EMPTY = {
    '\xcf', '\x83', '\xe1', '\x35', '\x7e', '\xef', '\xb8', '\xbd',
    '\xf1', '\x54', '\x28', '\x50', '\xd6', '\x6d', '\x80', '\x07',
    '\xd6', '\x20', '\xe4', '\x05', '\x0b', '\x57', '\x15', '\xdc',
    '\x83', '\xf4', '\xa9', '\x21', '\xd3', '\x6c', '\xe9', '\xce',
    '\x47', '\xd0', '\xd1', '\x3c', '\x5d', '\x85', '\xf2', '\xb0',
    '\xff', '\x83', '\x18', '\xd2', '\x87', '\x7e', '\xec', '\x2f',
    '\x63', '\xb9', '\x31', '\xbd', '\x47', '\x41', '\x7a', '\x81',
    '\xa5', '\x38', '\x32', '\x7a', '\xf9', '\x27', '\xda', '\x3e'
};
const std::vector<unsigned char> SHA_512_EMPTY_UNSIGNED(
        SHA_512_EMPTY.begin(), SHA_512_EMPTY.end());

const std::string HELLO = "hello";
// echo -n hello | openssl sha512
const std::vector<char> SHA_512_HELLO = {
    '\x9b', '\x71', '\xd2', '\x24', '\xbd', '\x62', '\xf3', '\x78',
    '\x5d', '\x96', '\xd4', '\x6a', '\xd3', '\xea', '\x3d', '\x73',
    '\x31', '\x9b', '\xfb', '\xc2', '\x89', '\x0c', '\xaa', '\xda',
    '\xe2', '\xdf', '\xf7', '\x25', '\x19', '\x67', '\x3c', '\xa7',
    '\x23', '\x23', '\xc3', '\xd9', '\x9b', '\xa5', '\xc1', '\x1d',
    '\x7c', '\x7a', '\xcc', '\x6e', '\x14', '\xb8', '\xc5', '\xda',
    '\x0c', '\x46', '\x63', '\x47', '\x5c', '\x2e', '\x5c', '\x3a',
    '\xde', '\xf4', '\x6f', '\x73', '\xbc', '\xde', '\xc0', '\x43'
};
const std::vector<unsigned char> SHA_512_HELLO_UNSIGNED(
        SHA_512_HELLO.begin(), SHA_512_HELLO.end());

}

class HashVerifyingFilter : public KulloTest {};

K_TEST_F(HashVerifyingFilter, doesntModifyData)
{
    std::vector<unsigned char> output;

    auto uut = make_unique<Crypto::HashVerifyingFilter>(SHA_512_HELLO_UNSIGNED);
    Util::FilterChain stream(make_unique<Util::BackInsertingSink>(output));
    stream.pushFilter(std::move(uut));

    stream.write(reinterpret_cast<const unsigned char *>(HELLO.data()),
                 HELLO.size());
    stream.close();

    EXPECT_THAT(output, Eq(Util::to_vector(HELLO)));
}

K_TEST_F(HashVerifyingFilter, doesntThrowForCorrectEmptyInput)
{
    auto uut = make_unique<Crypto::HashVerifyingFilter>(SHA_512_EMPTY_UNSIGNED);
    Util::FilterChain stream(make_unique<Util::NullSink>());
    stream.pushFilter(std::move(uut));

    EXPECT_NO_THROW(stream.close());
}

K_TEST_F(HashVerifyingFilter, doesntThrowForCorrectNonemptyInput)
{
    auto uut = make_unique<Crypto::HashVerifyingFilter>(SHA_512_HELLO_UNSIGNED);
    Util::FilterChain stream(make_unique<Util::NullSink>());
    stream.pushFilter(std::move(uut));

    stream.write(reinterpret_cast<const unsigned char *>(HELLO.data()),
                 HELLO.size());
    EXPECT_NO_THROW(stream.close());
}

K_TEST_F(HashVerifyingFilter, throwsOnBadHashLength)
{
    EXPECT_THROW(Crypto::HashVerifyingFilter({1, 2, 3}), Util::AssertionFailed);
}

K_TEST_F(HashVerifyingFilter, throwsOnWrongHashAndEmptyInput)
{
    auto uut = make_unique<Crypto::HashVerifyingFilter>(SHA_512_HELLO_UNSIGNED);

    Util::FilterChain stream(make_unique<Util::NullSink>());
    stream.pushFilter(std::move(uut));

    EXPECT_THROW(stream.close(), Crypto::HashDoesntMatch);
}

K_TEST_F(HashVerifyingFilter, throwsOnWrongHashAndNonemptyInput)
{
    std::string goodbye = "goodbye";
    auto uut = make_unique<Crypto::HashVerifyingFilter>(SHA_512_HELLO_UNSIGNED);

    Util::FilterChain stream(make_unique<Util::NullSink>());
    stream.pushFilter(std::move(uut));

    stream.write(reinterpret_cast<const unsigned char *>(goodbye.data()),
                 goodbye.length());
    EXPECT_THROW(stream.close(), Crypto::HashDoesntMatch);
}
