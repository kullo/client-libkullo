/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/messagedao.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Dao {

typedef Result<ConversationDao> ConversationResult;

/**
 * @brief Data Access Object for conversations
 */
class ConversationDao
{
public:
    /**
     * @brief Create a new conversation.
     */
    explicit ConversationDao(Db::SharedSessionPtr session);

    static std::unique_ptr<ConversationDao> load(id_type id, Db::SharedSessionPtr session);
    static std::unique_ptr<ConversationDao> load(std::set<Util::KulloAddress> participants, Db::SharedSessionPtr session);
    static std::unique_ptr<ConversationResult> all(Db::SharedSessionPtr session);

    std::string loadLastMessageTime();
    std::size_t loadMessageCount(const MessagesFilter filter);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    void deletePermanently();

    /// Get the conversation ID
    id_type id() const;

    /// Get the participant addresses, ordered alphabetically, concatenated by ','
    std::string participants() const;

    /// Get a list of the participant addresses, ordered alphabetically
    std::vector<std::string> participantsList() const;

    /**
     * @brief Set the participants
     * @param participants Format as in participants().
     *                     Must not include the local user!
     */
    bool setParticipants(const std::string &participants);

    /// Set the participants
    bool setParticipants(const std::set<Util::KulloAddress> &participants);

private:
    static std::string makeParticipantsString(const std::set<Util::KulloAddress> &participants);
    static std::unique_ptr<ConversationDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    bool dirty_ = true, storedInDb_ = false, deleted_ = false;
    id_type id_ = 0;
    std::string participants_;
    Db::SharedSessionPtr session_;

    friend class Result<ConversationDao>;
};

}
}
