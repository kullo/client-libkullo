/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <vector>
#include <kulloclient/util/gzipdecompressingfilter.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"
#include "tests/testdata.h"

using namespace Kullo;
using namespace GZipData;
using namespace testing;

class GZipDecompressingFilter : public KulloTest
{
};

K_TEST_F(GZipDecompressingFilter, decompressCompressedEmptyData)
{
    std::vector<unsigned char> output;
    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Util::GZipDecompressingFilter>());

    chain.write(EMTPY_GZIPPED);
    chain.close();
    EXPECT_THAT(output, IsEmpty());
}

K_TEST_F(GZipDecompressingFilter, decompressesEmptyInputToEmptyOutput)
{
    std::vector<unsigned char> output;
    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Util::GZipDecompressingFilter>());

    chain.write(nullptr, 0);
    chain.close();
    EXPECT_THAT(output, IsEmpty());
}

K_TEST_F(GZipDecompressingFilter, decompressHelloWorld)
{
    std::vector<unsigned char> output;
    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Util::GZipDecompressingFilter>());

    chain.write(HELLO_WORLD_GZIPPED);
    chain.close();
    EXPECT_THAT(output, Eq(HELLO_WORLD));
}

K_TEST_F(GZipDecompressingFilter, decompressHelloWorldInTwoWrites)
{
    std::vector<unsigned char> output;
    Util::FilterChain chain(make_unique<Util::BackInsertingSink>(output));
    chain.pushFilter(make_unique<Util::GZipDecompressingFilter>());

    std::size_t offset = 5;
    chain.write(HELLO_WORLD_GZIPPED.data(), offset);
    chain.write(HELLO_WORLD_GZIPPED.data() + offset,
                HELLO_WORLD_GZIPPED.size() - offset);
    chain.close();
    EXPECT_THAT(output, Eq(HELLO_WORLD));
}

K_TEST_F(GZipDecompressingFilter, decompressInvalidData)
{
    std::vector<unsigned char> invalidGzipData = {'A', 'B', 'C'};
    Util::FilterChain chain(make_unique<Util::NullSink>());
    chain.pushFilter(make_unique<Util::GZipDecompressingFilter>());

    EXPECT_ANY_THROW(chain.write(invalidGzipData));
}
