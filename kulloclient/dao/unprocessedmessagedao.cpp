/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/unprocessedmessagedao.h"

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Dao {

UnprocessedMessageDao::UnprocessedMessageDao(const Util::KulloAddress &sender, Db::SharedSessionPtr session)
    : session_(session),
      sender_(sender)
{
    kulloAssert(session);
}

std::unique_ptr<UnprocessedMessageDao> UnprocessedMessageDao::load(id_type id, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages_unprocessed "
                "WHERE id = :id");
    stmt.bind(":id", id);

    return UnprocessedMessageResult(std::move(stmt), session).next();
}

std::unique_ptr<UnprocessedMessageDao> UnprocessedMessageDao::loadFirst(Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages_unprocessed "
                "ORDER BY last_modified ASC "
                "LIMIT 1");

    return UnprocessedMessageResult(std::move(stmt), session).next();
}

std::unique_ptr<UnprocessedMessageDao> UnprocessedMessageDao::loadFirstForSignatureKey(const Util::KulloAddress &address, id_type sigKeyId, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages_unprocessed "
                "WHERE sender = :sender AND signature_key_id = :signature_key_id "
                "ORDER BY last_modified ASC "
                "LIMIT 1");
    stmt.bind(":sender", address.toString());
    stmt.bind(":signature_key_id", sigKeyId);

    return UnprocessedMessageResult(std::move(stmt), session).next();
}

void UnprocessedMessageDao::deleteAllForSignatureKey(const Util::KulloAddress &address, id_type sigKeyId, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "DELETE FROM messages_unprocessed "
                "WHERE sender = :sender AND signature_key_id = :signature_key_id");
    stmt.bind(":sender", address.toString());
    stmt.bind(":signature_key_id", sigKeyId);
    stmt.execWithoutResult();
}

bool UnprocessedMessageDao::save()
{
    if (!dirty_) return false;

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO messages_unprocessed VALUES ("
              ":id, :sender, :signature_key_id, :last_modified, "
              ":deleted, :received, :meta, :key_safe, :content, "
              ":has_attachments)";
    }
    else
    {
        sql = "UPDATE messages_unprocessed "
              "SET sender = :sender, signature_key_id = :signature_key_id, "
              "last_modified = :last_modified, deleted = :deleted, "
              "received = :received, meta = :meta, key_safe = :key_safe, "
              "content = :content, has_attachments = :has_attachments "
              "WHERE id = :id";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":id", msg_.id);
    stmt.bind(":sender", sender_.toString());
    stmt.bind(":signature_key_id", sigKeyId_);
    stmt.bind(":last_modified", msg_.lastModified);
    stmt.bind(":deleted", msg_.deleted);
    stmt.bind(":received", msg_.dateReceived->toString());
    stmt.bind(":meta", msg_.meta);
    stmt.bind(":key_safe", msg_.keySafe);
    stmt.bind(":content", msg_.content);
    stmt.bind(":has_attachments", msg_.hasAttachments);
    stmt.execWithoutResult();

    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void UnprocessedMessageDao::deletePermanently()
{
    auto stmt = session_->prepare("DELETE FROM messages_unprocessed WHERE id = :id");
    stmt.bind(":id", msg_.id);
    stmt.execWithoutResult();

    storedInDb_ = false;
    dirty_ = true;
}

Util::KulloAddress UnprocessedMessageDao::sender() const
{
    return sender_;
}

bool UnprocessedMessageDao::setSender(const Util::KulloAddress &sender)
{
    return assignAndUpdateDirty(sender_, sender, dirty_);
}

id_type UnprocessedMessageDao::signatureKeyId() const
{
    return sigKeyId_;
}

bool UnprocessedMessageDao::setSignatureKeyId(id_type sigKeyId)
{
    return assignAndUpdateDirty(sigKeyId_, sigKeyId, dirty_);
}

Protocol::Message UnprocessedMessageDao::message() const
{
    return msg_;
}

bool UnprocessedMessageDao::setMessage(const Protocol::Message &msg)
{
    // id is the primary key and thus may not change
    kulloAssert(msg_.id == 0 || msg_.id == msg.id);
    return assignAndUpdateDirty(msg_, msg, dirty_);
}

std::unique_ptr<UnprocessedMessageDao> UnprocessedMessageDao::loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session)
{
    try
    {
        Protocol::Message msg;
        msg.id = row.get<id_type>("id");
        auto sender = Util::KulloAddress(row.get<std::string>("sender"));
        auto dao = make_unique<UnprocessedMessageDao>(sender, session);
        dao->sigKeyId_     = row.get<id_type>("signature_key_id");
        msg.lastModified   = row.get<lastModified_type>("last_modified");
        msg.deleted        = row.get<bool>("deleted");
        msg.dateReceived   = Util::DateTime(row.get<std::string>("received"));
        msg.meta           = row.get<std::vector<unsigned char>>("meta");
        msg.keySafe        = row.get<std::vector<unsigned char>>("key_safe");
        msg.content        = row.get<std::vector<unsigned char>>("content");
        msg.hasAttachments = row.get<bool>("has_attachments");

        dao->msg_ = msg;
        dao->dirty_ = false;
        dao->storedInDb_ = true;
        return dao;
    }
    catch (...)
    {
        std::throw_with_nested(Db::DatabaseIntegrityError("Failed to load UnprocessedMessageDao"));
        return nullptr;  // silence "missing return" warning
    }
}

}
}
