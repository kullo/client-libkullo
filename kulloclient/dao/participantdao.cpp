/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/participantdao.h"

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

namespace {
// for each address that is a sender:
//     select the participant record from the newest message
const std::string SQL_SELECT_SENDERS =
        "WITH latest_senders AS ( "
        "  SELECT id, MAX(received) "
        "  FROM messages "
        "  WHERE sender != '' AND old = 0 "
        "  GROUP BY sender "
        ") "
        "SELECT p.* "
        "FROM participants p "
        "JOIN latest_senders ON p.message_id = latest_senders.id";
}

ParticipantDao::ParticipantDao(const KulloAddress &address, Db::SharedSessionPtr session)
    : session_(session),
      address_(address)
{
    kulloAssert(session);
}

std::unique_ptr<ParticipantDao> ParticipantDao::load(id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM participants WHERE message_id = :message_id");
    stmt.bind(":message_id", messageId);

    return ParticipantResult(std::move(stmt), session).next();
}

std::unique_ptr<ParticipantDao> ParticipantDao::load(
        const Util::KulloAddress &address, SharedSessionPtr session)
{
    kulloAssert(session);

    std::string sql;
    sql = SQL_SELECT_SENDERS + " WHERE p.address = :address";
    auto stmt = session->prepare(sql);
    stmt.bind(":address", address.toString());
    return ParticipantResult(std::move(stmt), session).next();
}

std::unique_ptr<ParticipantResult> ParticipantDao::latestSenders(SharedSessionPtr session)
{
    kulloAssert(session);

    std::string sql = SQL_SELECT_SENDERS;
    auto stmt = session->prepare(sql);

    return make_unique<ParticipantResult>(std::move(stmt), session);
}

std::unique_ptr<ParticipantResult> ParticipantDao::all(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare("SELECT * FROM participants p "
                                 "ORDER BY message_id");
    return make_unique<ParticipantResult>(std::move(stmt), session);
}

bool ParticipantDao::save()
{
    if (!dirty_) return false;

    kulloAssert(!storedInDb_); // UPDATE has not been implemented

    auto stmt = session_->prepare(
                "INSERT INTO participants VALUES ("
                ":message_id, :name, :address, :organization, "
                ":avatar_mime_type, :avatar)");

    stmt.bind(":message_id", messageId_);
    stmt.bind(":name", name_);
    stmt.bind(":address", address_.toString());
    stmt.bind(":organization", organization_);
    stmt.bind(":avatar_mime_type", avatarMimeType_);
    stmt.bind(":avatar", avatar_);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void ParticipantDao::deletePermanently()
{
    auto stmt = session_->prepare("DELETE FROM participants WHERE message_id = :message_id");
    stmt.bind(":message_id", messageId_);
    stmt.execWithoutResult();

    storedInDb_ = false;
    dirty_ = true;
}

id_type ParticipantDao::messageId() const
{
    return messageId_;
}

bool ParticipantDao::setMessageId(id_type messageId)
{
    if (assignAndUpdateDirty(messageId_, messageId, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

std::string ParticipantDao::name() const
{
    return name_;
}

bool ParticipantDao::setName(const std::string &name)
{
    return assignAndUpdateDirty(name_, name, dirty_);
}

KulloAddress ParticipantDao::address() const
{
    return address_;
}

bool ParticipantDao::setAddress(const KulloAddress &address)
{
    return assignAndUpdateDirty(address_, address, dirty_);
}

std::string ParticipantDao::organization() const
{
    return organization_;
}

bool ParticipantDao::setOrganization(const std::string &organization)
{
    return assignAndUpdateDirty(organization_, organization, dirty_);
}

std::string ParticipantDao::avatarMimeType() const
{
    return avatarMimeType_;
}

bool ParticipantDao::setAvatarMimeType(const std::string &avatarMimeType)
{
    return assignAndUpdateDirty(avatarMimeType_, avatarMimeType, dirty_);
}

std::vector<unsigned char> ParticipantDao::avatar() const
{
    return avatar_;
}

bool ParticipantDao::setAvatar(const std::vector<unsigned char> &avatar)
{
    return assignAndUpdateDirty(avatar_, avatar, dirty_);
}

std::unique_ptr<ParticipantDao> ParticipantDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto address = KulloAddress(row.get<std::string>("address"));
    auto dao = make_unique<ParticipantDao>(address, session);

    dao->messageId_ = row.get<id_type>("message_id");
    dao->name_ = row.get<std::string>("name");
    dao->organization_ = row.get<std::string>("organization");
    dao->avatarMimeType_ = row.get<std::string>("avatar_mime_type");
    dao->avatar_ = row.get<std::vector<unsigned char>>("avatar");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
