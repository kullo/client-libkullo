/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/api/Address.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

K_TEST(ApiAddressStatic, createFailsForInvalidAddress)
{
    EXPECT_THAT(Api::Address::create(""), IsNull());
    EXPECT_THAT(Api::Address::create("invalid@address"), IsNull());
}

K_TEST(ApiAddressStatic, createWorks)
{
    EXPECT_THAT(Api::Address::create("example#kullo.net"), Not(IsNull()));
}


class ApiAddress : public KulloTest
{
protected:
    ApiAddress()
        : uut(Api::Address::create(addr))
    {}

    std::string addr = "example#kullo.net";
    std::shared_ptr<Api::Address> uut;
};

K_TEST_F(ApiAddress, toStringWorks)
{
    EXPECT_THAT(uut->toString(), Eq(addr));
}

K_TEST_F(ApiAddress, localPartWorks)
{
    EXPECT_THAT(uut->localPart(), Eq("example"));
}

K_TEST_F(ApiAddress, domainPartWorks)
{
    EXPECT_THAT(uut->domainPart(), Eq("kullo.net"));
}

K_TEST_F(ApiAddress, isEqualToWorks)
{
    auto uut2 =  Api::Address::create(addr);
    EXPECT_THAT(uut->isEqualTo(uut2), Eq(true));

    auto uut3 =  Api::Address::create("other#kullo.net");
    EXPECT_THAT(uut->isEqualTo(uut3), Eq(false));
}

K_TEST_F(ApiAddress, isLessThanWorks)
{
    auto uut2 =  Api::Address::create(addr);
    EXPECT_THAT(uut->isLessThan(uut2), Eq(false));

    auto uut3 =  Api::Address::create("other#kullo.net");
    EXPECT_THAT(uut->isLessThan(uut3), Eq(true));
}
