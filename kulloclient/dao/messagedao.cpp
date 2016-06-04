/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/messagedao.h"

#include <ostream>

#include <smartsqlite/scopedsavepoint.h>

#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/daoutil.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

MessageDao::MessageDao(SharedSessionPtr session)
    : session_(session)
{
    kulloAssert(session);
}

std::unique_ptr<MessageDao> MessageDao::load(id_type id, Old old, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages WHERE id = :id AND old = :old");
    stmt.bind(":id", id);
    stmt.bind(":old", (old == Old::Yes) ? 1 : 0);

    return MessageResult(std::move(stmt), session).next();
}

std::map<MessageState, bool> MessageDao::loadState(id_type messageId, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT read, done FROM messages WHERE id = :id AND old = 0");
    stmt.bind(":id", messageId);

    auto row = stmt.execWithSingleResult();
    auto read = row.get<bool>("read");
    auto done = row.get<bool>("done");

    auto out = std::map<MessageState, bool>();
    out[MessageState::Read] = read;
    out[MessageState::Done] = done;
    return out;
}

std::vector<id_type> MessageDao::locallyModifiedMessages(metaVersion_type maxMetaVersion, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    std::vector<id_type> ids;

    auto stmt = session->prepare("SELECT id FROM messages WHERE old = 1 AND meta_version <= :max_meta_version");
    stmt.bind(":max_meta_version", maxMetaVersion);
    for (auto row : stmt)
    {
        id_type id = row.get<id_type>(0);
        ids.push_back(id);
    }

    return ids;
}

std::unique_ptr<MessageResult> MessageDao::messagesForConversation(id_type conversationId, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages "
                "WHERE conversation_id = :conversation_id AND old = 0 AND deleted = 0");
    stmt.bind(":conversation_id", conversationId);

    return make_unique<MessageResult>(std::move(stmt), session);
}

std::unique_ptr<MessageResult> MessageDao::all(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM messages "
                "WHERE old = 0 "
                "ORDER BY id ASC");

    return make_unique<MessageResult>(std::move(stmt), session);
}

bool MessageDao::operator==(const MessageDao &other) const
{
    // Notice that we only compare the identity and syncable contents

    if (this->id_ != other.id_) return false;

    // objects are equal if they are both deleted, no matter what the other fields are
    if (this->deleted_ && other.deleted_) return true;

    return
            this->deleted_ == other.deleted_ &&
            this->metaVersion_ == other.metaVersion_ &&
            this->read_ == other.read_ &&
            this->done_ == other.done_;
}

bool MessageDao::operator!=(const MessageDao &other) const
{
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &out, const MessageDao &dao)
{
    std::map<std::string, std::string> data;
    data["id"]             = std::to_string(dao.id_);
    data["conversationId"] = std::to_string(dao.conversationId_);
    data["metaVersion"]    = std::to_string(dao.metaVersion_);
    data["deleted"] = dao.deleted_ ? "true" : "false";
    data["read"]    = dao.read_ ? "true" : "false";
    data["done"]    = dao.done_ ? "true" : "false";
    data["sender"]  = "\"" + dao.sender_ + "\"";
    data["text"]    = "\"" + dao.text_ + "\"";
    data["footer"]  = "\"" + dao.footer_ + "\"";

    out << "MessageDao(";
    bool first = true;
    for (const auto &row : data)
    {
        if (!first) out << "; ";
        out << row.first << ": " << row.second;
        first = false;
    }
    out << ")";
    return out;
}

bool MessageDao::save(CreateOld createOldIfDoesntExist)
{
    if (!dirty_)
    {
        return false;
    }

    const std::string savepointName = "messagedao_" + std::to_string(id_);
    SmartSqlite::ScopedSavepoint sp(session_, savepointName);

    // if already synced: insert copy of current record (not state of this!) with old = true
    if (createOldIfDoesntExist == CreateOld::Yes)
    {
        bool oldExists;
        {
            auto stmt = session_->prepare(
                        "SELECT id FROM messages WHERE id = :id AND old = 1");
            stmt.bind(":id", id_);
            oldExists = stmt.hasResults();
        }

        if (!oldExists)
        {
            auto stmt = session_->prepare(
                        "INSERT INTO messages "
                        "SELECT id, conversation_id, sender, "
                        "last_modified, deleted, meta_version, read, done, "
                        "sent, received, text, footer, symmetric_key, 1 "
                        "FROM messages WHERE id = :id");
            stmt.bind(":id", id_);
            stmt.execWithoutResult();
        }
    }

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO messages VALUES ("
                ":id, :conversation_id, :sender, :last_modified, "
                ":deleted, :meta_version, :read, :done, :sent, :received, "
                ":text, :footer, :symmetric_key, :old)";
    }
    else
    {
        sql = "UPDATE messages SET "
                "conversation_id = :conversation_id, "
                "sender = :sender, "
                "last_modified = :last_modified, "
                "deleted = :deleted, "
                "meta_version = :meta_version, "
                "read = :read, "
                "done = :done, "
                "sent = :sent, "
                "received = :received, "
                "text = :text, "
                "footer = :footer, "
                "symmetric_key = :symmetric_key "
                "WHERE id = :id AND old = :old";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":id", id_);
    stmt.bind(":conversation_id", conversationId_);
    stmt.bind(":sender", sender_);
    stmt.bind(":last_modified", lastModified_);
    stmt.bind(":deleted", deleted_);
    stmt.bind(":meta_version", metaVersion_);
    stmt.bind(":read", read_);
    stmt.bind(":done", done_);
    stmt.bind(":sent", dateSent_);
    stmt.bind(":received", dateReceived_);
    stmt.bind(":text", text_);
    stmt.bind(":footer", footer_);
    if (symmetricKey_.size()) stmt.bind(":symmetric_key", symmetricKey_);
    else stmt.bindNull(":symmetric_key");
    stmt.bind(":old", old_);
    stmt.execWithoutResult();

    sp.release();
    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void MessageDao::clearData()
{
    setConversationId(0);
    setSender("");
    setDateSent("");
    setDateReceived("");
    setText("");
    setFooter("");
    setSymmetricKey({});
    setMetaVersion(0);
    setState(MessageState::Read, false);
    setState(MessageState::Done, false);
}

bool MessageDao::old() const
{
    return old_;
}

void MessageDao::dropOld() const
{
    kulloAssert(!old_); // Can only drop old message if this instance is not old

    auto stmt = session_->prepare(
                "DELETE FROM messages WHERE id = :id AND old = 1");
    stmt.bind(":id", id_);
    stmt.execWithoutResult();
}

id_type MessageDao::id() const
{
    return id_;
}

bool MessageDao::setId(id_type id)
{
    if (assignAndUpdateDirty(id_, id, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

id_type MessageDao::conversationId() const
{
    return conversationId_;
}

bool MessageDao::setConversationId(id_type conversationId)
{
    return assignAndUpdateDirty(conversationId_, conversationId, dirty_);
}

lastModified_type MessageDao::lastModified() const
{
    return lastModified_;
}

bool MessageDao::setLastModified(lastModified_type lastModified)
{
    return assignAndUpdateDirty(lastModified_, lastModified, dirty_);
}

std::string MessageDao::sender() const
{
    return sender_;
}

bool MessageDao::setSender(const std::string &sender)
{
    return assignAndUpdateDirty(sender_, sender, dirty_);
}

bool MessageDao::deleted() const
{
    return deleted_;
}

bool MessageDao::setDeleted(bool deleted)
{
    return assignAndUpdateDirty(deleted_, deleted, dirty_);
}

std::string MessageDao::dateSent() const
{
    return dateSent_;
}

bool MessageDao::setDateSent(const std::string &dateSent)
{
    return assignAndUpdateDirty(dateSent_, dateSent, dirty_);
}

std::string MessageDao::dateReceived() const
{
    return dateReceived_;
}

bool MessageDao::setDateReceived(const std::string &dateReceived)
{
    return assignAndUpdateDirty(dateReceived_, dateReceived, dirty_);
}

std::string MessageDao::text() const
{
    return text_;
}

bool MessageDao::setText(const std::string &text)
{
    return assignAndUpdateDirty(text_, text, dirty_);
}

std::string MessageDao::footer() const
{
    return footer_;
}

bool MessageDao::setFooter(const std::string &footer)
{
    return assignAndUpdateDirty(footer_, footer, dirty_);
}

std::vector<unsigned char> MessageDao::symmetricKey() const
{
    return symmetricKey_;
}

bool MessageDao::setSymmetricKey(const std::vector<unsigned char> &symmetricKey)
{
    return assignAndUpdateDirty(symmetricKey_, symmetricKey, dirty_);
}

bool MessageDao::state(MessageState state) const
{
    switch (state) {
    case MessageState::Read:
        return read_;
    case MessageState::Unread:
        return !read_;
    case MessageState::Done:
        return done_;
    case MessageState::Undone:
        return !done_;
    case MessageState::Any:
        return true;
    default:
        kulloAssert(false); // unknown state
        return false;
    }
}

bool MessageDao::setState(MessageState state, bool value)
{
    switch (state) {
    case MessageState::Read:
        return assignAndUpdateDirty(read_, value, dirty_);

    case MessageState::Done:
        return assignAndUpdateDirty(done_, value, dirty_);

    default:
        kulloAssert(false); // unknown state
        return false;
    }
}

metaVersion_type MessageDao::metaVersion() const
{
    return metaVersion_;
}

bool MessageDao::setMetaVersion(metaVersion_type metaVersion)
{
    return assignAndUpdateDirty(metaVersion_, metaVersion, dirty_);
}

std::unique_ptr<MessageDao> MessageDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<MessageDao>(session);
    dao->id_ = row.get<id_type>("id");
    dao->conversationId_ = row.get<id_type>("conversation_id");
    dao->sender_ = row.get<std::string>("sender");
    dao->lastModified_ = row.get<lastModified_type>("last_modified");
    dao->deleted_ = row.get<bool>("deleted");
    dao->metaVersion_ = row.get<metaVersion_type>("meta_version");
    dao->read_ = row.get<bool>("read");
    dao->done_ = row.get<bool>("done");
    dao->dateSent_ = row.get<std::string>("sent");
    dao->dateReceived_ = row.get<std::string>("received");
    dao->text_ = row.get<std::string>("text");
    dao->footer_ = row.get<std::string>("footer");
    dao->symmetricKey_ = row.get<std::vector<unsigned char>>("symmetric_key");
    dao->old_ = row.get<bool>("old");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
