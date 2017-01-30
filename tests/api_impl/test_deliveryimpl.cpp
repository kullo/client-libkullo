/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <kulloclient/api/Delivery.h>
#include <kulloclient/api_impl/deliveryimpl.h>

#include <kulloclient/util/kulloaddress.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class ApiImplDelivery : public KulloTest
{
public:
    ApiImplDelivery()
    {
        data.adress1 = std::make_shared<Util::KulloAddress>("recipient1#kullo.net");
        data.adress2 = std::make_shared<Util::KulloAddress>("recipient2#kullo.net");
    }

protected:
    struct {
        std::shared_ptr<Util::KulloAddress> adress1;
        std::shared_ptr<Util::KulloAddress> adress2;
    } data;
};

K_TEST_F(ApiImplDelivery, operatorEquals)
{
    using State = Util::Delivery::State;

    {
        auto d1 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::unsent));
        auto d2 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::unsent));
        EXPECT_THAT(d1, Eq(d2));
    }

    {
        auto d1 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::delivered));
        auto d2 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::delivered));
        EXPECT_THAT(d1, Eq(d2));
    }

    {
        auto d1 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress2, State::delivered));
        auto d2 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress2, State::delivered));
        EXPECT_THAT(d1, Eq(d2));
    }

    {
        // adress differs
        auto d1 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::unsent));
        auto d2 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress2, State::unsent));
        EXPECT_THAT(d1, Ne(d2));
    }

    {
        // state differs
        auto d1 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::unsent));
        auto d2 = ApiImpl::DeliveryImpl(Util::Delivery(*data.adress1, State::delivered));
        EXPECT_THAT(d1, Ne(d2));
    }

    {
        // failed reason differs: only one has reason
        auto d1_raw = Util::Delivery(*data.adress1, State::failed);
        auto d2_raw = Util::Delivery(*data.adress1, State::failed);
        d1_raw.reason = boost::make_optional<Util::Delivery::Reason>(Util::Delivery::Reason::canceled);
        auto d1 = ApiImpl::DeliveryImpl(d1_raw);
        auto d2 = ApiImpl::DeliveryImpl(d2_raw);
        EXPECT_THAT(d1, Ne(d2));
    }

    {
        // failed reason differs: reason differs
        auto d1_raw = Util::Delivery(*data.adress1, State::failed);
        auto d2_raw = Util::Delivery(*data.adress1, State::failed);
        d1_raw.reason = boost::make_optional<Util::Delivery::Reason>(Util::Delivery::Reason::canceled);
        d2_raw.reason = boost::make_optional<Util::Delivery::Reason>(Util::Delivery::Reason::doesnt_exist);
        auto d1 = ApiImpl::DeliveryImpl(d1_raw);
        auto d2 = ApiImpl::DeliveryImpl(d2_raw);
        EXPECT_THAT(d1, Ne(d2));
    }
}

K_TEST_F(ApiImplDelivery, deliveryListsEqual)
{
    auto d1 = std::make_shared<ApiImpl::DeliveryImpl>(Util::Delivery(*data.adress1, Util::Delivery::State::unsent));
    auto d2 = std::make_shared<ApiImpl::DeliveryImpl>(Util::Delivery(*data.adress1, Util::Delivery::State::delivered));
    auto d3_raw = Util::Delivery(*data.adress1, Util::Delivery::State::failed);
    d3_raw.reason = boost::make_optional<Util::Delivery::Reason>(Util::Delivery::Reason::doesnt_exist);
    auto d3 = std::make_shared<ApiImpl::DeliveryImpl>(d3_raw);

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>();
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>();
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(true));
    }

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>{ d1 };
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>{ d1 };
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(true));
    }

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>{ d1, d2 };
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>{ d1, d2 };
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(true));
    }

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>{ d1, d2, d3 };
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>{ d1, d2, d3 };
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(true));
    }

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>{ d1 };
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>{ d2 };
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(false));
    }

    {
        auto l1 = std::vector<std::shared_ptr<Api::Delivery>>{ d1, d2 };
        auto l2 = std::vector<std::shared_ptr<Api::Delivery>>{ d2, d1 };
        EXPECT_THAT(ApiImpl::DeliveryImpl::deliveryListsEqual(l1, l2), Eq(false));
    }
}
