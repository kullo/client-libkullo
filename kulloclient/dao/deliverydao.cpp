/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/deliverydao.h"

#include <utility>
#include <smartsqlite/connection.h>

#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Dao {

DeliveryDao::DeliveryDao(Db::SharedSessionPtr session) :
    session_(session)
{
    kulloAssert(session);
}

namespace {
Util::Delivery readDeliveryRecord(const SmartSqlite::Row &row)
{
    Util::KulloAddress recipient(row.get<std::string>("recipient"));
    auto state = Util::Delivery::toState(row.get<std::string>("state"));
    auto reason = row.getNullable<std::string>("reason");
    auto date = row.getNullable<std::string>("date");
    auto lockHolder = row.getNullable<std::string>("lock_holder");
    auto lockExpires = row.getNullable<std::string>("lock_expires");

    Util::Delivery del(recipient, state);
    if (reason) del.reason = Util::Delivery::toReason(*reason);
    if (date) del.date = Util::DateTime(*date);
    if (lockHolder) del.lockHolder = *lockHolder;
    if (lockExpires) del.lockExpires = Util::DateTime(*lockExpires);
    return del;
}
}

std::vector<Util::Delivery> DeliveryDao::loadDelivery(id_type msgId)
{
    auto stmt = session_->prepare(
                "SELECT recipient, state, reason, date, lock_holder, lock_expires "
                "FROM delivery "
                "WHERE message_id = :messageId "
                "ORDER BY recipient ");
    stmt.bind(":messageId", msgId);

    std::vector<Util::Delivery> result;
    for (const auto &row : stmt)
    {
        result.emplace_back(readDeliveryRecord(row));
    }
    return result;
}

std::vector<std::pair<id_type, Util::Delivery>> DeliveryDao::loadAll()
{
    auto stmt = session_->prepare(
                "SELECT message_id, recipient, state, reason, date, lock_holder, lock_expires "
                "FROM delivery "
                "ORDER BY message_id, recipient ");

    std::vector<std::pair<id_type, Util::Delivery>> result;
    for (const auto &row : stmt)
    {
        auto msgId = row.get<id_type>("message_id");
        result.emplace_back(msgId, readDeliveryRecord(row));
    }
    return result;
}

void DeliveryDao::insertDelivery(id_type msgId, const std::vector<Util::Delivery> &delivery)
{
    for (const auto &del : delivery)
    {
        auto stmt = session_->prepare(
                    "INSERT INTO delivery VALUES ("
                    ":messageId, :recipient, :state, :reason, "
                    ":date, :lockHolder, :lockExpires)");
        stmt.bind(":messageId", msgId);
        stmt.bind(":recipient", del.recipient.toString());
        stmt.bind(":state", Util::Delivery::toString(del.state));
        switch (del.state)
        {
        case Util::Delivery::delivered:
            kulloAssert(del.date);
            stmt.bindNull(":reason");
            stmt.bind(":date", del.date->toString());
            stmt.bindNull(":lockHolder");
            stmt.bindNull(":lockExpires");
            break;
        case Util::Delivery::failed:
            kulloAssert(del.reason);
            kulloAssert(del.date);
            stmt.bind(":reason", Util::Delivery::toString(*del.reason));
            stmt.bind(":date", del.date->toString());
            stmt.bindNull(":lockHolder");
            stmt.bindNull(":lockExpires");
            break;
        case Util::Delivery::unsent:
            stmt.bindNull(":reason");
            stmt.bindNull(":date");
            if (del.lockHolder && del.lockExpires)
            {
                stmt.bind(":lockHolder", *del.lockHolder);
                stmt.bind(":lockExpires", del.lockExpires->toString());
            }
            else
            {
                stmt.bindNull(":lockHolder");
                stmt.bindNull(":lockExpires");
            }
            break;
        default:
            kulloAssert(false);
        }

        stmt.execWithoutResult();
    }
}

void DeliveryDao::markAsDelivered(
        id_type msgId,
        const Util::KulloAddress &recipient,
        const Util::DateTime &date)
{
    auto stmt = session_->prepare(
                "UPDATE delivery "
                "SET state = :state, reason = :reason, date = :date, "
                "lock_holder = :lockHolder, lock_expires = :lockExpires "
                "WHERE message_id = :messageId AND recipient = :recipient ");
    stmt.bind(":state", Util::Delivery::toString(Util::Delivery::delivered));
    stmt.bindNull(":reason");
    stmt.bind(":date", date.toString());
    stmt.bindNull(":lockHolder");
    stmt.bindNull(":lockExpires");
    stmt.bind(":messageId", msgId);
    stmt.bind(":recipient", recipient.toString());

    stmt.execWithoutResult();
}

void DeliveryDao::markAsFailed(
        id_type msgId,
        const Util::KulloAddress &recipient,
        const Util::DateTime &date,
        Util::Delivery::Reason reason)
{
    auto stmt = session_->prepare(
                "UPDATE delivery "
                "SET state = :state, reason = :reason, date = :date, "
                "lock_holder = :lockHolder, lock_expires = :lockExpires "
                "WHERE message_id = :messageId AND recipient = :recipient ");
    stmt.bind(":state", Util::Delivery::toString(Util::Delivery::failed));
    stmt.bind(":reason", Util::Delivery::toString(reason));
    stmt.bind(":date", date.toString());
    stmt.bindNull(":lockHolder");
    stmt.bindNull(":lockExpires");
    stmt.bind(":messageId", msgId);
    stmt.bind(":recipient", recipient.toString());

    stmt.execWithoutResult();
}

void DeliveryDao::remove(id_type msgId)
{
    auto stmt = session_->prepare(
                "DELETE FROM delivery "
                "WHERE message_id = :messageId ");
    stmt.bind(":messageId", msgId);
    stmt.execWithoutResult();
}

std::vector<DeliveryDao::IdAndAddress> DeliveryDao::unsentMessages()
{
    auto stmt = session_->prepare(
                "SELECT message_id, recipient "
                "FROM delivery "
                "WHERE state = :state "
                "ORDER BY message_id, recipient ");

    stmt.bind(":state", Util::Delivery::toString(Util::Delivery::unsent));

    std::vector<IdAndAddress> result;
    for (const auto &row : stmt)
    {
        result.emplace_back(
                    row.get<id_type>("message_id"),
                    Util::KulloAddress(row.get<std::string>("recipient")));
    }
    return result;
}

}
}
