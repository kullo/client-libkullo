/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/dao/enums.h"
#include "kulloclient/dao/result.h"

namespace Kullo {
namespace Dao {

typedef Result<MessageDao> MessageResult;

/**
 * @brief Data Access Object for messages
 */
class MessageDao
{
public:
    /**
     * @brief Create a new message.
     */
    explicit MessageDao(Db::SharedSessionPtr session);

    /**
     * @brief Load a message from DB.
     * @param id ID of the message
     * @param old Whether to load the version of last sync if there were changes.
     *            Only used for syncing, set to false otherwise.
     */
    static std::unique_ptr<MessageDao> load(id_type id, Old old, Db::SharedSessionPtr session);

    static std::map<MessageState, bool> loadState(id_type messageId, Db::SharedSessionPtr session);

    static std::vector<id_type> locallyModifiedMessages(metaVersion_type maxMetaVersion,
                                                            Db::SharedSessionPtr session);
    static std::unique_ptr<MessageResult> messagesForConversation(id_type conversationId,
                                                  Db::SharedSessionPtr session);

    static std::unique_ptr<MessageResult> all(Db::SharedSessionPtr session);
    static std::size_t sizeOfAllUndelivered(Db::SharedSessionPtr session);

    bool operator==(const MessageDao &other) const;
    bool operator!=(const MessageDao &other) const;
    friend std::ostream &operator<<(std::ostream &out, const MessageDao &dao);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save(CreateOld createOldIfDoesntExist);

    /// Clears all fields but id, old, lastModified and deleted.
    void clearData();

    /// Get the old bit (only used for syncing)
    bool old() const;

    /**
     * @brief Drop the old message version for this non-old message (only used for syncing)
     * @throw MessageException if this->old is true
     */
    void dropOld() const;

    /// Get the message ID (assigned by server)
    id_type id() const;

    bool setId(id_type id);

    /// Get the conversation ID
    id_type conversationId() const;

    /// Set the conversation ID
    bool setConversationId(id_type conversationId);

    /// Get the modification timestamp (assigned by server)
    lastModified_type lastModified() const;

    /// Set the modification timestamp
    bool setLastModified(lastModified_type lastModified);

    /// Get the sender address
    std::string sender() const;

    /// Set the sender address
    bool setSender(const std::string &sender);

    /// true iff the message was deleted
    bool deleted() const;

    /**
     * @brief Delete or undelete the message.
     *
     * When a message is deleted, it is only marked as deleted, so that
     * undo is possible.
     */
    bool setDeleted(bool deleted);

    /// Get the sent date (assigned by sender)
    std::string dateSent() const;

    /// Set the sent date
    bool setDateSent(const std::string &sent);

    /// Get the received date (assigned by server)
    std::string dateReceived() const;

    /// Set the received date
    bool setDateReceived(const std::string &received);

    /// Get the text body
    std::string text() const;

    /// Set the text body
    bool setText(const std::string &text);

    /// Get the footer text
    std::string footer() const;

    /// Set the footer text
    bool setFooter(const std::string &footer);

    std::vector<unsigned char> symmetricKey() const;

    bool setSymmetricKey(const std::vector<unsigned char> &symmetricKey);

    /**
     * @brief Get the message state
     * @throw MessageException if the state is unknown or invalid
     */
    bool state(MessageState state) const;

    /**
     * @brief Set the message state
     * @throw MessageException if the state is unknown or invalid
     */
    bool setState(MessageState state, bool value);

    metaVersion_type metaVersion() const;
    bool setMetaVersion(metaVersion_type metaVersion);

private:
    static std::unique_ptr<MessageDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool old_ = false, deleted_ = false, read_ = false, done_ = false;
    bool dirty_ = true, storedInDb_ = false;
    id_type id_ = 0, conversationId_ = 0;
    lastModified_type lastModified_ = 0;
    std::string sender_, text_, footer_;
    std::string dateSent_, dateReceived_;
    std::vector<unsigned char> symmetricKey_;
    metaVersion_type metaVersion_ = 0;

    friend class Result<MessageDao>;
};

}
}
