/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/api_impl/Address.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class ApiAddress : public KulloTest
{
protected:
    const std::string addressString_ = "example#kullo.net";
    const Api::Address uut_ = addressString_;
};

K_TEST_F(ApiAddress, componentwiseConstructorWorks)
{
    EXPECT_THAT(uut_, Eq(Api::Address("example", "kullo.net")));
    EXPECT_THAT(uut_, Ne(Api::Address("examplx", "kullo.net")));
    EXPECT_THAT(uut_, Ne(Api::Address("example", "kullo.org")));

    EXPECT_ANY_THROW(Api::Address("", "kullo.net"));
    EXPECT_ANY_THROW(Api::Address(" example", "kullo.net"));
    EXPECT_ANY_THROW(Api::Address("example ", "kullo.net"));
    EXPECT_ANY_THROW(Api::Address("exampleä", "kullo.net"));

    EXPECT_ANY_THROW(Api::Address("example", ""));
    EXPECT_ANY_THROW(Api::Address("example", " kullo.net"));
    EXPECT_ANY_THROW(Api::Address("example", "kullo.net "));
    EXPECT_ANY_THROW(Api::Address("example", "kullo.netö"));
}

K_TEST_F(ApiAddress, toStringWorks)
{
    EXPECT_THAT(uut_.toString(), Eq(addressString_));
}

K_TEST_F(ApiAddress, localPartWorks)
{
    EXPECT_THAT(uut_.localPart, Eq("example"));
}

K_TEST_F(ApiAddress, domainPartWorks)
{
    EXPECT_THAT(uut_.domainPart, Eq("kullo.net"));
}

K_TEST_F(ApiAddress, isEqualToWorks)
{
    auto address2 = Api::Address(addressString_);
    EXPECT_THAT(uut_, Eq(address2));

    auto address3 = Api::Address("other#kullo.net");
    EXPECT_THAT(uut_, Ne(address3));
}
