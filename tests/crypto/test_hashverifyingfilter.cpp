/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/null.hpp>
#include <kulloclient/crypto/exceptions.h>
#include <kulloclient/crypto/hashverifyingfilter.h>
#include <kulloclient/util/binary.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

namespace io = boost::iostreams;

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

class HashVerifyingFilter : public KulloTest
{
protected:
    io::filtering_ostream stream_;
};

K_TEST_F(HashVerifyingFilter, doesntModifyData)
{
    std::vector<char> output(HELLO.size());

    Crypto::HashVerifyingFilter uut(SHA_512_HELLO_UNSIGNED);
    stream_.push(boost::ref(uut));
    stream_.push(io::array_sink(output.data(), output.size()));
    stream_.write(HELLO.data(), static_cast<std::streamsize>(HELLO.size()));
    stream_.pop();

    EXPECT_THAT(output, Eq(std::vector<char>(HELLO.begin(), HELLO.end())));
}

K_TEST_F(HashVerifyingFilter, calculatesHashForEmptyInput)
{
    Crypto::HashVerifyingFilter uut(SHA_512_EMPTY_UNSIGNED);
    stream_.push(boost::ref(uut));
    stream_.push(io::null_sink());
    stream_.pop();

    EXPECT_THAT(uut.hash(), Eq(SHA_512_EMPTY_UNSIGNED));
}

K_TEST_F(HashVerifyingFilter, calculatesHashForNonemptyInput)
{
    Crypto::HashVerifyingFilter uut(SHA_512_HELLO_UNSIGNED);
    stream_.push(boost::ref(uut));
    stream_.push(io::null_sink());
    stream_.write(HELLO.data(), static_cast<std::streamsize>(HELLO.size()));
    stream_.pop();

    EXPECT_THAT(uut.hash(), Eq(SHA_512_HELLO_UNSIGNED));
}

K_TEST_F(HashVerifyingFilter, throwsOnWrongHash)
{
    std::string goodbye = "goodbye";

    Crypto::HashVerifyingFilter uut(SHA_512_HELLO_UNSIGNED);
    stream_.push(boost::ref(uut));
    stream_.push(io::null_sink());
    stream_.write(goodbye.data(), static_cast<std::streamsize>(goodbye.length()));
    EXPECT_THROW(stream_.pop(), Crypto::HashDoesntMatch);
}
