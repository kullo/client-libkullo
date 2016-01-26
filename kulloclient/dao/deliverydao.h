/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

#include "kulloclient/dao/enums.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/delivery.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Dao {

class DeliveryDao
{
public:
    DeliveryDao(Db::SharedSessionPtr session);

    std::vector<Util::Delivery> loadDelivery(id_type msgId);
    std::vector<std::pair<id_type, Util::Delivery>> loadAll();

    void insertDelivery(id_type msgId, const std::vector<Util::Delivery> &delivery);

    void markAsDelivered(
            id_type msgId,
            const Util::KulloAddress &recipient,
            const Util::DateTime &date);

    void markAsFailed(
            id_type msgId,
            const Util::KulloAddress &recipient,
            const Util::DateTime &date,
            Util::Delivery::Reason reason);

    void remove(id_type msgId);

    using IdAndAddress = std::tuple<id_type, Util::KulloAddress>;
    std::vector<IdAndAddress> unsentMessages();

private:
    Db::SharedSessionPtr session_;
};

}
}
