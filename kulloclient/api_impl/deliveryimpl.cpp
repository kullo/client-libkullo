/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/deliveryimpl.h"

#include "kulloclient/api/DeliveryState.h"
#include "kulloclient/api/DeliveryReason.h"
#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

DeliveryImpl::DeliveryImpl(const Util::Delivery &delivery)
    : delivery_(delivery)
{}

Api::Address DeliveryImpl::recipient()
{
    return Api::Address(delivery_.recipient);
}

Api::DeliveryState DeliveryImpl::state()
{
    switch (delivery_.state)
    {
    case Util::Delivery::unsent:
        return Api::DeliveryState::Unsent;
    case Util::Delivery::delivered:
        return Api::DeliveryState::Delivered;
    case Util::Delivery::failed:
        return Api::DeliveryState::Failed;
    }
}

boost::optional<Api::DeliveryReason> DeliveryImpl::reason()
{
    if (!delivery_.reason) return {};

    switch (*delivery_.reason)
    {
    case Util::Delivery::unknown:
        return Api::DeliveryReason::Unknown;
    case Util::Delivery::doesnt_exist:
        return Api::DeliveryReason::DoesntExist;
    case Util::Delivery::too_large:
        return Api::DeliveryReason::TooLarge;
    case Util::Delivery::canceled:
        return Api::DeliveryReason::Canceled;
    }
}

boost::optional<Api::DateTime> DeliveryImpl::date()
{
    auto &date = delivery_.date;
    return date ? boost::make_optional(Api::DateTime(*date)) : boost::none;
}

bool DeliveryImpl::operator==(const DeliveryImpl &other) const
{
    return delivery_ == other.delivery_;
}

bool DeliveryImpl::operator!=(const DeliveryImpl &other) const
{
    return !(*this == other);
}

bool DeliveryImpl::deliveryListsEqual(std::vector<nn_shared_ptr<Api::Delivery>> list1,
                                      std::vector<nn_shared_ptr<Api::Delivery>> list2)
{
    if (list1.size() != list2.size()) return false;

    for (unsigned int pos = 0; pos < list1.size(); ++pos)
    {
        auto item1 = std::dynamic_pointer_cast<DeliveryImpl>(list1[pos].as_nullable());
        kulloAssert(item1);
        auto item2 = std::dynamic_pointer_cast<DeliveryImpl>(list2[pos].as_nullable());
        kulloAssert(item2);

        if (*item1 != *item2) return false;
    }

    return true;
}

}
}
