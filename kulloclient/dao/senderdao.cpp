/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/dao/senderdao.h"

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

SenderDao::SenderDao(const KulloAddress &address, Db::SharedSessionPtr session)
    : session_(session),
      address_(address)
{
    kulloAssert(session);
}

std::unique_ptr<SenderDao> SenderDao::load(id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM senders WHERE message_id = :message_id");
    stmt.bind(":message_id", messageId);

    return SenderResult(std::move(stmt), session).next();
}

std::unique_ptr<SenderResult> SenderDao::all(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare("SELECT * FROM senders s "
                                 "ORDER BY message_id");
    return make_unique<SenderResult>(std::move(stmt), session);
}

bool SenderDao::save()
{
    if (!dirty_) return false;

    kulloAssert(!storedInDb_); // UPDATE has not been implemented

    kulloAssertMsg(messageId_ >= ID_MIN && messageId_ <= ID_MAX,
                   "Sender with invalid message ID must not be stored in database.");

    auto stmt = session_->prepare(
                "INSERT INTO senders VALUES ("
                ":message_id, :name, :address, :organization, "
                ":avatar_mime_type, :avatar_hash)");

    stmt.bind(":message_id", messageId_);
    stmt.bind(":name", name_);
    stmt.bind(":address", address_.toString());
    stmt.bind(":organization", organization_);
    stmt.bind(":avatar_mime_type", avatarMimeType_);
    if (avatarHash())
    {
        stmt.bind(":avatar_hash", *avatarHash_);
    }
    else
    {
        stmt.bindNull(":avatar_hash");
    }
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void SenderDao::deletePermanently()
{
    auto stmt = session_->prepare("DELETE FROM senders WHERE message_id = :message_id");
    stmt.bind(":message_id", messageId_);
    stmt.execWithoutResult();

    storedInDb_ = false;
    dirty_ = true;
}

id_type SenderDao::messageId() const
{
    return messageId_;
}

bool SenderDao::setMessageId(id_type messageId)
{
    if (assignAndUpdateDirty(messageId_, messageId, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

std::string SenderDao::name() const
{
    return name_;
}

bool SenderDao::setName(const std::string &name)
{
    return assignAndUpdateDirty(name_, name, dirty_);
}

KulloAddress SenderDao::address() const
{
    return address_;
}

std::string SenderDao::organization() const
{
    return organization_;
}

bool SenderDao::setOrganization(const std::string &organization)
{
    return assignAndUpdateDirty(organization_, organization, dirty_);
}

std::string SenderDao::avatarMimeType() const
{
    return avatarMimeType_;
}

bool SenderDao::setAvatarMimeType(const std::string &avatarMimeType)
{
    return assignAndUpdateDirty(avatarMimeType_, avatarMimeType, dirty_);
}

boost::optional<int64_t> SenderDao::avatarHash() const
{
    return avatarHash_;
}

bool SenderDao::setAvatarHash(boost::optional<std::int64_t> avatarHash)
{
    return assignAndUpdateDirty(avatarHash_, avatarHash, dirty_);
}

std::unique_ptr<SenderDao> SenderDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto address = KulloAddress(row.get<std::string>("address"));
    auto dao = make_unique<SenderDao>(address, session);

    dao->messageId_ = row.get<id_type>("message_id");
    dao->name_ = row.get<std::string>("name");
    dao->organization_ = row.get<std::string>("organization");
    dao->avatarMimeType_ = row.get<std::string>("avatar_mime_type");
    auto hash = row.getNullable<std::int64_t>("avatar_hash");
    dao->avatarHash_ = (hash) ? boost::make_optional<std::int64_t>(*hash) : boost::none;
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
