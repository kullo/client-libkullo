/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/draftdao.h"

#include <smartsqlite/scopedsavepoint.h>

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

DraftDao::DraftDao(SharedSessionPtr session)
    : session_(session)
{
}

std::unique_ptr<DraftDao> DraftDao::load(id_type conversationId, Db::SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM drafts "
                "WHERE conversation_id = :conversation_id");
    stmt.bind(":conversation_id", conversationId);

    return DraftResult(std::move(stmt), session).next();
}

std::unique_ptr<DraftDao> DraftDao::firstSendable(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM drafts "
                "WHERE state = :state "
                "LIMIT 1");
    stmt.bind(":state", static_cast<int>(DraftState::Sending));

    return DraftResult(std::move(stmt), session).next();
}

std::unique_ptr<DraftResult> DraftDao::all(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare(
                "SELECT * FROM drafts "
                "ORDER BY conversation_id ASC");

    return make_unique<DraftResult>(std::move(stmt), session);
}

std::size_t DraftDao::sizeOfAllSendable(SharedSessionPtr session)
{
    kulloAssert(session);
    // If a bug is found in here, chances are that it is also in
    // MessageDao::sizeOfAllUndelivered.

    // size of draft = (text size + attachment size) * number of recipients
    // Number of participants is number of commas in `participants`,
    // + 1 for the last entry which isn't succeeded by a comma,
    // + 1 for the sender, who also gets a copy.
    auto stmt = session->prepare(
                // calculate map conversation -> size of its draft attachments
                "WITH attsizes (message_id, total_size) AS ( "
                "    SELECT message_id, sum(size) "
                "    FROM attachments "
                "    GROUP BY message_id "
                "    HAVING draft = 1 "
                ") "

                "SELECT "
                // return a row (with 0) even when there are no sendable drafts
                "coalesce(sum( "
                "    (:msg_overhead + length(d.text) + coalesce(a.total_size, 0)) "
                "    * ( "
                // commas = total chars - chars which are not a comma
                "        length(c.participants) "
                "        - length(replace(c.participants, ',', '')) "
                "        + 2 "
                "    ) "
                "), 0) "
                "FROM drafts d, conversations c LEFT JOIN attsizes a "
                "ON c.id = a.message_id "
                "WHERE d.conversation_id = c.id AND d.state = :state ");
    stmt.bind(":msg_overhead", PER_MESSAGE_OVERHEAD);
    stmt.bind(":state", static_cast<int>(DraftState::Sending));
    return stmt.execWithSingleResult().get<std::size_t>(0);
}

bool DraftDao::save()
{
    if (!dirty_)
    {
        return false;
    }

    const std::string savepointName = "draftdao_" + std::to_string(conversationId_);
    SmartSqlite::ScopedSavepoint sp(session_, savepointName);

    std::string sql;
    if (!storedInDb_)
    {
        sql = "INSERT INTO drafts VALUES ("
                ":id, :conversation_id, :last_modified, "
                ":text, :footer, :sender_name, :sender_organization, "
                ":sender_avatar, :sender_avatar_mime_type, "
                ":state, :old)";
    }
    else
    {
        sql = "UPDATE drafts "
                "SET id = :id, last_modified = :last_modified, "
                "text = :text, footer = :footer, "
                "sender_name = :sender_name, sender_organization = :sender_organization, "
                "sender_avatar = :sender_avatar, sender_avatar_mime_type = :sender_avatar_mime_type, "
                "state = :state "
                "WHERE conversation_id = :conversation_id AND old = :old";
    }
    auto stmt = session_->prepare(sql);
    stmt.bind(":id", id_);
    stmt.bind(":conversation_id", conversationId_);
    stmt.bind(":last_modified", lastModified_);
    stmt.bind(":text", text_);
    stmt.bind(":footer", footer_);
    stmt.bind(":sender_name", senderName_);
    stmt.bind(":sender_organization", senderOrganization_);
    if (senderAvatar_.empty()) stmt.bindNull(":sender_avatar");
    else stmt.bind(":sender_avatar", senderAvatar_);
    stmt.bind(":sender_avatar_mime_type", senderAvatarMimeType_);
    stmt.bind(":state", static_cast<int>(state_));
    stmt.bind(":old", false);
    stmt.execWithoutResult();

    sp.release();
    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void DraftDao::clear()
{
    setState(DraftState::Editing);
    setText("");
    setFooter("");
    setSenderName("");
    setSenderOrganization("");
    setSenderAvatar(std::vector<unsigned char>());
    setSenderAvatarMimeType("");
}

id_type DraftDao::id() const
{
    return id_;
}

bool DraftDao::setId(id_type id)
{
    return assignAndUpdateDirty(id_, id, dirty_);
}

id_type DraftDao::conversationId() const
{
    return conversationId_;
}

bool DraftDao::setConversationId(id_type conversationId)
{
    if (assignAndUpdateDirty(conversationId_, conversationId, dirty_))
    {
        storedInDb_ = false;
        return true;
    }
    return false;
}

lastModified_type DraftDao::lastModified() const
{
    return lastModified_;
}

bool DraftDao::setLastModified(lastModified_type lastModified)
{
    return assignAndUpdateDirty(lastModified_, lastModified, dirty_);
}

std::string DraftDao::text() const
{
    return text_;
}

bool DraftDao::setText(const std::string &text)
{
    return assignAndUpdateDirty(text_, text, dirty_);
}

std::string DraftDao::footer() const
{
    return footer_;
}

bool DraftDao::setFooter(const std::string &footer)
{
    return assignAndUpdateDirty(footer_, footer, dirty_);
}

std::string DraftDao::senderName() const
{
    return senderName_;
}

bool DraftDao::setSenderName(const std::string &senderName)
{
    return assignAndUpdateDirty(senderName_, senderName, dirty_);
}

std::string DraftDao::senderOrganization() const
{
    return senderOrganization_;
}

bool DraftDao::setSenderOrganization(const std::string &senderOrganization)
{
    return assignAndUpdateDirty(senderOrganization_, senderOrganization, dirty_);
}

std::vector<unsigned char> DraftDao::senderAvatar() const
{
    return senderAvatar_;
}

bool DraftDao::setSenderAvatar(const std::vector<unsigned char> &data)
{
    return assignAndUpdateDirty(senderAvatar_, data, dirty_);
}

std::string DraftDao::senderAvatarMimeType() const
{
    return senderAvatarMimeType_;
}

bool DraftDao::setSenderAvatarMimeType(const std::string &mime)
{
    return assignAndUpdateDirty(senderAvatarMimeType_, mime, dirty_);
}

DraftState DraftDao::state() const
{
    return state_;
}

bool DraftDao::setState(DraftState state)
{
    return assignAndUpdateDirty(state_, state, dirty_);
}

DraftState DraftDao::intToDraftState(int stateInt)
{
    switch (static_cast<DraftState>(stateInt))
    {
    case DraftState::Editing:
        return DraftState::Editing;
    case DraftState::Sending:
        return DraftState::Sending;
    }
}

std::unique_ptr<DraftDao> DraftDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<DraftDao>(session);
    dao->id_                   = row.get<id_type>("id");
    dao->conversationId_       = row.get<id_type>("conversation_id");
    dao->lastModified_         = row.get<lastModified_type>("last_modified");
    dao->text_                 = row.get<std::string>("text");
    dao->footer_               = row.get<std::string>("footer");
    dao->senderName_           = row.get<std::string>("sender_name");
    dao->senderOrganization_   = row.get<std::string>("sender_organization");
    dao->senderAvatar_         = row.get<std::vector<unsigned char>>("sender_avatar");
    dao->senderAvatarMimeType_ = row.get<std::string>("sender_avatar_mime_type");
    dao->state_                = intToDraftState(row.get<int>("state"));
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
