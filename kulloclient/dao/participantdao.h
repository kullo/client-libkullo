/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/enums.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Dao {

typedef Result<ParticipantDao> ParticipantResult;

/**
 * @brief Data Access Object for participants
 */
class ParticipantDao
{
public:
    /**
     * @brief Create a new participant.
     */
    explicit ParticipantDao(const Util::KulloAddress &address, Db::SharedSessionPtr session);

    static std::unique_ptr<ParticipantDao> load(id_type messageId, Db::SharedSessionPtr session);

    /**
     * @brief Load a participant from DB.
     * @param address Address of the participant
     */
    static std::unique_ptr<ParticipantDao> load(
            const Util::KulloAddress &address, Db::SharedSessionPtr session);

    /**
     * @brief Returns an executed query for all participants.
     *
     * For every address, if there are sender records for the address, the newest will be included.
     * Otherwise, the only (non-sender) record is included.
     */
    static std::unique_ptr<ParticipantResult> latestSenders(Db::SharedSessionPtr session);
    static std::unique_ptr<ParticipantResult> all(Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    /**
     * @brief Deletes a participant if it is a sender for a message.
     *
     * Should be used when a message was deleted.
     */
    void deletePermanently();

    /**
     * @brief Get the message ID of which the participant is the sender
     * @return The message ID, or 0 iff this is not a sender record
     */
    id_type messageId() const;

    /// Set the message id
    bool setMessageId(id_type messageId);

    /// Get the real name (e.g. "John Doe")
    std::string name() const;

    /// Set the real name
    bool setName(const std::string &name);

    /// Get the address (e.g. "john.doe#kullo.net")
    Util::KulloAddress address() const;

    /// Set the address
    bool setAddress(const Util::KulloAddress &address);

    /// Get the organization name (e.g. "Doe Inc.")
    std::string organization() const;

    /// Set the organization name
    bool setOrganization(const std::string &organization);

    /// Get the MIME type of the avatar (e.g. "image/png")
    std::string avatarMimeType() const;

    /// Set the MIME type of the avatar
    bool setAvatarMimeType(const std::string &avatarMimeType);

    /// Get the binary avatar file contents
    std::vector<unsigned char> avatar() const;

    /// Set the binary avatar file contents
    bool setAvatar(const std::vector<unsigned char> &avatar);

private:
    static std::unique_ptr<ParticipantDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true, storedInDb_ = false;
    id_type messageId_ = 0;
    Util::KulloAddress address_;
    std::string name_, organization_;
    std::vector<unsigned char> avatar_;
    std::string avatarMimeType_;

    friend class Result<ParticipantDao>;
};

}
}
