/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/codec/exceptions.h>
#include <kulloclient/codec/sizelimitingfilter.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class SizeLimitingFilter : public KulloTest
{
public:
    SizeLimitingFilter()
        : input_(Util::to_vector("Hello"))
    {}

protected:
    const std::vector<unsigned char> input_;
};

K_TEST_F(SizeLimitingFilter, doesntModifyData)
{
    std::vector<unsigned char> output;

    auto uut = make_unique<Codec::SizeLimitingFilter>(input_.size());
    Util::FilterChain stream(make_unique<Util::BackInsertingSink>(output));
    stream.pushFilter(std::move(uut));

    stream.write(input_.data(), input_.size());
    stream.close();

    EXPECT_THAT(output, Eq(input_));
}

K_TEST_F(SizeLimitingFilter, limitsSize)
{
    std::vector<unsigned char> output;

    auto uut = make_unique<Codec::SizeLimitingFilter>(input_.size() - 1);
    Util::FilterChain stream(make_unique<Util::BackInsertingSink>(output));
    stream.pushFilter(std::move(uut));

    EXPECT_THROW(stream.write(input_.data(), input_.size()),
                 Codec::InvalidContentFormat);
}
