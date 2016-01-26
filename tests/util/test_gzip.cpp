/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <vector>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/gzip.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class GZip : public KulloTest
{
};

namespace {

const std::vector<unsigned char> HELLO_WORLD =
        Util::to_vector("Hello World\n");

// echo "Hello World" | gzip | hexdump -C
const std::vector<unsigned char> HELLO_WORLD_GZIPPED = {
        0x1f, 0x8b, 0x08, 0x00, 0xd6, 0x02, 0x87, 0x54,
        0x00, 0x03, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0x57,
        0x08, 0xcf, 0x2f, 0xca, 0x49, 0xe1, 0x02, 0x00,
        0xe3, 0xe5, 0x95, 0xb0, 0x0c, 0x00, 0x00, 0x00
};

// echo -n "" | gzip | hexdump -C
const std::vector<unsigned char> EMTPY_GZIPPED = {
        0x1f, 0x8b, 0x08, 0x00, 0xad, 0x06, 0x87, 0x54,
        0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};

}

K_TEST_F(GZip, compressShortText)
{
    auto compressed = Util::GZip::compress(HELLO_WORLD);
    EXPECT_THAT(compressed, Not(IsEmpty()));
}

K_TEST_F(GZip, compressLongText)
{
    auto original = std::string(
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            "Hello, world. Hello, world. Hello, world. Hello, world. "
            );
    auto compressed = Util::GZip::compress(Util::to_vector(original));
    EXPECT_THAT(compressed, Not(IsEmpty()));
    EXPECT_THAT(compressed.size(), Lt(0.5 * original.size()));
}

K_TEST_F(GZip, compressEmptyData)
{
    auto compressed = Util::GZip::compress({});
    EXPECT_THAT(compressed, Not(IsEmpty()));

    // gzip header >= 18 bytes
    // http://www.onicos.com/staff/iz/formats/gzip.html
    EXPECT_THAT(compressed.size(), Ge(size_t{18}));
}

K_TEST_F(GZip, decompressShortText)
{
    auto decompressed = Util::GZip::decompress(HELLO_WORLD_GZIPPED);
    EXPECT_THAT(decompressed, Eq(HELLO_WORLD));
}

K_TEST_F(GZip, decompressEmptyData)
{
    auto decompressed = Util::GZip::decompress(EMTPY_GZIPPED);
    EXPECT_THAT(decompressed, IsEmpty());
}

K_TEST_F(GZip, decompressInvalidData)
{
    auto invalidGzipData = std::vector<unsigned char>{'A', 'B', 'C'};
    EXPECT_ANY_THROW(Util::GZip::decompress(invalidGzipData));
}

K_TEST_F(GZip, compressAndDecompressEmptyData)
{
    auto compressed = Util::GZip::compress(std::vector<unsigned char>{});
    auto decompressed = Util::GZip::decompress(compressed);
    EXPECT_THAT(decompressed, Eq(std::vector<unsigned char>{}));
}

K_TEST_F(GZip, compressAndDecompressShortText)
{
    auto compressed = Util::GZip::compress(HELLO_WORLD);
    auto decompressed = Util::GZip::decompress(compressed);
    EXPECT_THAT(decompressed, Eq(HELLO_WORLD));
}
