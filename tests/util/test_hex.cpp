/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/hex.h>
#include <kulloclient/util/binary.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class Hex : public KulloTest
{
};

K_TEST_F(Hex, encodeAndDecodeEmptyString)
{
    // common knowledge
    EXPECT_THAT(Util::Hex::encode(""), StrEq(""));
    EXPECT_THAT(Util::Hex::decode(""), Eq(Util::to_vector("")));
}

K_TEST_F(Hex, encodeShortString)
{
    EXPECT_THAT(Util::Hex::encode("f"), StrEq("66"));
    EXPECT_THAT(Util::Hex::encode("fo"), StrEq("666f"));
    EXPECT_THAT(Util::Hex::encode("foo"), StrEq("666f6f"));
}

K_TEST_F(Hex, decodeShortString)
{
    EXPECT_THAT(Util::Hex::decode("66"),     Eq(Util::to_vector("f")));
    EXPECT_THAT(Util::Hex::decode("666f"),   Eq(Util::to_vector("fo")));
    EXPECT_THAT(Util::Hex::decode("666f6f"), Eq(Util::to_vector("foo")));
}

K_TEST_F(Hex, encodeDecodeSingleByte)
{
    for (unsigned char c = 0; c != 255; ++c)
    {
        SCOPED_TRACE(int{c});

        auto encoded = Util::Hex::encode(std::vector<unsigned char>{ c });
        EXPECT_THAT(encoded.length(), Eq(size_t{2}));

        auto decoded = Util::Hex::decode(encoded);
        EXPECT_THAT(decoded.size(), Eq(size_t{1}));
        EXPECT_THAT(decoded[0], Eq(c));
    }
}
