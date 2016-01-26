/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/conversationdao.h"

#include <algorithm>
#include <sstream>

#include "kulloclient/dao/daoutil.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Db;
using namespace Kullo::Util;

namespace Kullo {
namespace Dao {

ConversationDao::ConversationDao(SharedSessionPtr session)
    : session_(session)
{
    kulloAssert(session);
}

std::unique_ptr<ConversationDao> ConversationDao::load(id_type id, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare("SELECT * FROM conversations WHERE id = :id");
    stmt.bind(":id", id);

    return ConversationResult(std::move(stmt), session).next();
}

std::unique_ptr<ConversationDao> ConversationDao::load(std::set<KulloAddress> participants, SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare("SELECT * FROM conversations WHERE participants = :participants");
    stmt.bind(":participants", makeParticipantsString(participants));

    return ConversationResult(std::move(stmt), session).next();
}

std::unique_ptr<ConversationResult> ConversationDao::all(SharedSessionPtr session)
{
    kulloAssert(session);

    auto stmt = session->prepare("SELECT * FROM conversations ORDER BY id ASC");

    return make_unique<ConversationResult>(std::move(stmt), session);
}

std::string ConversationDao::loadLastMessageTime()
{
    std::string sql = "SELECT coalesce(max(strftime('%Y-%m-%dT%H:%M:%SZ', sent)), '') AS last_message_time "
                      "FROM messages "
                      "WHERE conversation_id = :conversation_id AND deleted = 0 AND old = 0";
    auto stmt = session_->prepare(sql);
    stmt.bind(":conversation_id", id_);
    SmartSqlite::Row row = stmt.execWithSingleResult();
    return row.get<std::string>("last_message_time");
}

std::size_t ConversationDao::loadMessageCount(MessageState type)
{
    std::string sqlWherePart;
    switch (type) {
    case MessageState::Any:
        sqlWherePart = "";
        break;
    case MessageState::Read:
        sqlWherePart = "AND read = 1";
        break;
    case MessageState::Unread:
        sqlWherePart = "AND read = 0";
        break;
    case MessageState::Done:
        sqlWherePart = "AND done = 1";
        break;
    case MessageState::Undone:
        sqlWherePart = "AND done = 0";
        break;
    default:
        kulloAssert(false);
    }

    const std::string sql = "SELECT count(id) AS count "
                            "FROM messages "
                            "WHERE conversation_id = :conversation_id "
                            "AND deleted = 0 AND old = 0 " + sqlWherePart;
    auto stmt = session_->prepare(sql);
    stmt.bind(":conversation_id", id_);
    SmartSqlite::Row row = stmt.execWithSingleResult();
    return row.get<std::size_t>("count");
}

bool ConversationDao::save()
{
    if (!dirty_) return false;

    kulloAssert(!storedInDb_); // UPDATE has not been implemented
    kulloAssert(participants_.length() != 0);

    auto stmt = session_->prepare(
                "INSERT INTO conversations (participants) "
                "VALUES (:participants)");
    stmt.bind(":participants", participants_);
    stmt.execWithoutResult();

    id_ = static_cast<id_type>(session_->lastInsertRowId());
    storedInDb_ = true;
    dirty_ = false;
    return true;
}

void ConversationDao::deletePermanently()
{
    if (deleted_) return;

    auto stmt = session_->prepare(
                "DELETE FROM conversations WHERE id = :id");
    stmt.bind(":id", id_);
    stmt.execWithoutResult();

    deleted_ = true;
    storedInDb_ = false;
    dirty_ = true;
}

id_type ConversationDao::id() const
{
    return id_;
}

std::string ConversationDao::participants() const
{
    return participants_;
}

std::vector<std::string> ConversationDao::participantsList() const
{
    std::vector<std::string> result;
    std::stringstream partsStream(participants_);
    std::string part;
    while (std::getline(partsStream, part, ','))
    {
        result.push_back(part);
    }
    return result;
}

bool ConversationDao::setParticipants(const std::string &participants)
{
    return assignAndUpdateDirty(participants_, participants, dirty_);
}

bool ConversationDao::setParticipants(const std::set<KulloAddress> &participants)
{
    return setParticipants(makeParticipantsString(participants));
}

std::string ConversationDao::makeParticipantsString(const std::set<KulloAddress> &participants)
{
    std::vector<std::string> participantsList;
    for(auto p : participants) {
        participantsList.push_back(p.toString());
    }
    std::sort(participantsList.begin(), participantsList.end());

    std::stringstream strstream;
    std::copy(participantsList.cbegin(), participantsList.cend(),
              std::ostream_iterator<std::string>(strstream, ","));

    // remove trailing ','
    std::string resultStr = strstream.str();
    return resultStr.substr(0, resultStr.length() - 1);
}

std::unique_ptr<ConversationDao> ConversationDao::loadFromDb(const SmartSqlite::Row &row, SharedSessionPtr session)
{
    auto dao = make_unique<ConversationDao>(session);
    dao->id_ = row.get<id_type>("id");
    dao->participants_ = row.get<std::string>("participants");
    dao->dirty_ = false;
    dao->storedInDb_ = true;
    return dao;
}

}
}
