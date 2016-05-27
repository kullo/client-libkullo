/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/util/datetime.h"

namespace Kullo {
namespace Util {

struct Credentials
{
    Credentials(
            std::shared_ptr<KulloAddress> &&address,
            std::shared_ptr<MasterKey> &&masterKey);

    /// Kullo address of the local user (e.g. "john.doe#kullo.net")
    const std::shared_ptr<KulloAddress> address;

    /// The masterKey
    const std::shared_ptr<MasterKey> masterKey;
};

/**
 * @brief Contains the settings of the local user.
 */
struct UserSettings
{
    explicit UserSettings(Credentials &&credentials);

    /// Login credentials of the user
    const Credentials credentials;

    /// Real name of the local user (e.g. "John Doe")
    std::string name;

    /// Organization of the local user (e.g. "Doe Inc.")
    std::string organization;

    /// MIME type of the avatar (e.g. "image/jpeg")
    std::string avatarMimeType;

    /// The contents of the avatar image file of type avatarType
    std::vector<unsigned char> avatarData;

    /// The footer (e.g. "Firstname Lastname, Mainstreet 123")
    std::string footer;

    /**
     * @brief The next time when the user should be reminded of making a
     *        MasterKey backup.
     *
     * boost::none means that there's no reminder scheduled.
     */
    boost::optional<DateTime> nextMasterKeyBackupReminder = DateTime::epoch();
};

}
}
