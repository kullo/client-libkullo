/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/dao/enums.h"
#include "kulloclient/dao/result.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Dao {

using SenderResult = Result<SenderDao>;

/**
 * @brief Data Access Object for senders
 */
class SenderDao
{
public:
    /// Create a new sender
    explicit SenderDao(const Util::KulloAddress &address, Db::SharedSessionPtr session);

    static std::unique_ptr<SenderDao> load(id_type messageId, Db::SharedSessionPtr session);

    static std::unique_ptr<SenderResult> all(Db::SharedSessionPtr session);

    /**
     * @brief Saves the current state to DB if the state was changed.
     * @return true if state was dirty
     */
    bool save();

    /**
     * @brief Deletes a sender if it is a sender for a message.
     *
     * Should be used when a message was deleted.
     */
    void deletePermanently();

    /// Get the message ID of which the sender is the sender or
    /// undefined if not assigned to a message
    id_type messageId() const;

    /// Set the message id
    bool setMessageId(id_type messageId);

    /// Get the real name (e.g. "John Doe")
    std::string name() const;

    /// Set the real name
    bool setName(const std::string &name);

    /// Get the address (e.g. "john.doe#kullo.net")
    Util::KulloAddress address() const;

    /// Get the organization name (e.g. "Doe Inc.")
    std::string organization() const;

    /// Set the organization name
    bool setOrganization(const std::string &organization);

    /// Get the MIME type of the avatar (e.g. "image/png")
    std::string avatarMimeType() const;

    /// Set the MIME type of the avatar
    bool setAvatarMimeType(const std::string &avatarMimeType);

    /// Get the avatar hash
    boost::optional<std::int64_t> avatarHash() const;

    /// Set the avatar hash
    bool setAvatarHash(boost::optional<int64_t> avatarHash);

private:
    static std::unique_ptr<SenderDao> loadFromDb(const SmartSqlite::Row &row, Db::SharedSessionPtr session);

    Db::SharedSessionPtr session_;
    bool dirty_ = true;
    bool storedInDb_ = false;

    const Util::KulloAddress address_;
    id_type messageId_ = -1;
    std::string name_;
    std::string organization_;
    std::string avatarMimeType_;
    boost::optional<std::int64_t> avatarHash_;

    friend class Result<SenderDao>;
};

}
}
