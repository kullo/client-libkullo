/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/enums.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace Dao {

typedef Result<DraftDao> DraftResult;

/**
 * @brief Data Access Object for drafts
 */
class DraftDao
{
public:
    /**
     * @brief Create a new draft.
     */
    explicit DraftDao(Db::SharedSessionPtr session);

    /**
     * @brief Load a draft from DB or create a new one if it doesn't exist.
     * @param conversationId ID of the conversation to which the draft should belong.
     * @param old Whether to load the version of last sync if there were changes.
     *            Only used for syncing, set to false otherwise.
     */
    static std::unique_ptr<DraftDao> load(id_type conversationId, Db::SharedSessionPtr session);

    /**
     * @brief Get a draft from DB that has the state IDraft::STATE_SENDING.
     * @return The found draft, or 0 if none exist.
     */
    static std::unique_ptr<DraftDao> firstSendable(Db::SharedSessionPtr session);

    static std::unique_ptr<DraftResult> all(Db::SharedSessionPtr session);
    static std::size_t sizeOfAllSendable(Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    /**
     * @brief Resets the draft, setting the state to editing and deleting text and footer.
     *
     * Attachments must be deleted separately, see AttachmentDao::deleteAttachmentsForDraft()
     */
    void clear();

    /// Get the draft ID (assigned by server, 0 iff not synced yet)
    id_type id() const;

    /// Set the draft ID
    bool setId(id_type id);

    /// Get the conversation ID
    id_type conversationId() const;

    /// Set the conversation ID
    bool setConversationId(id_type conversationId);

    /// Get the modification timestamp (assigned by server)
    lastModified_type lastModified() const;

    /// Set the modification timestamp
    bool setLastModified(lastModified_type lastModified);

    /// Get the draft text body
    std::string text() const;

    /// Set the draft text body
    bool setText(const std::string &text);

    /// Get the draft footer text
    std::string footer() const;

    /// Set the draft footer text
    bool setFooter(const std::string &footer);

    std::string senderName() const;
    bool setSenderName(const std::string &senderName);

    std::string senderOrganization() const;
    bool setSenderOrganization(const std::string &senderOrganization);

    std::vector<unsigned char> senderAvatar() const;
    bool setSenderAvatar(const std::vector<unsigned char> &data);

    std::string senderAvatarMimeType() const;
    bool setSenderAvatarMimeType(const std::string &mime);

    /// Get the draft state
    DraftState state() const;

    /// Set the draft state
    bool setState(DraftState state);

private:
    static DraftState intToDraftState(int stateInt);
    static std::unique_ptr<DraftDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true;
    bool storedInDb_ = false;
    id_type id_ = 0, conversationId_ = 0;
    lastModified_type lastModified_ = 0;
    std::string text_, footer_;
    std::string senderName_;
    std::string senderOrganization_;
    std::vector<unsigned char> senderAvatar_;
    std::string senderAvatarMimeType_;
    DraftState state_ = DraftState::Editing;

    friend class Result<DraftDao>;
};

}
}
