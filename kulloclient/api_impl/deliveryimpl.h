/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <vector>
#include <memory>

#include "kulloclient/api/Delivery.h"
#include "kulloclient/util/delivery.h"
#include "kulloclient/nn.h"

namespace Kullo {
namespace ApiImpl {

class DeliveryImpl : public Api::Delivery
{
public:
    DeliveryImpl(const Util::Delivery &delivery);

    Api::Address recipient() override;
    Api::DeliveryState state() override;
    boost::optional<Api::DeliveryReason> reason() override;
    boost::optional<Api::DateTime> date() override;

    bool operator==(const DeliveryImpl &other) const;
    bool operator!=(const DeliveryImpl &other) const;

    // unsorted helper function that has no better place to live
    // We assume that delivery lists are sorted consistently
    static bool deliveryListsEqual(std::vector<nn_shared_ptr<Delivery> > list1,
                                   std::vector<nn_shared_ptr<Delivery> > list2);

private:
    Util::Delivery delivery_;
};

}
}
