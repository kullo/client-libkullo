/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/util/datetime.h"

namespace Kullo {
namespace Util {

/**
 * @brief Contains the settings of the local user.
 *
 * This class is passed to IClient::create() and is used for sending messages.
 * For details on the fields consult IParticipant.
 */
class UserSettings
{
public:
    /// Kullo address of the local user (e.g. "john.doe#kullo.net")
    std::shared_ptr<KulloAddress> address = nullptr;

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

    /// The masterKey
    std::shared_ptr<MasterKey> masterKey = nullptr;

    /// masterKeyBackupConfirmed
    bool masterKeyBackupConfirmed() const;
    void setMasterKeyBackupConfirmed();

    /// masterKeyBackupDontRemindBefore
    DateTime masterKeyBackupDontRemindBefore() const;
    void setMasterKeyBackupDontRemindBefore(const DateTime &dontRemindBefore);

    /// Reset everything to the same state as default constructed
    void reset();

private:
    bool masterKeyBackupConfirmed_ = false;
    DateTime masterKeyBackupDontRemindBefore_ = DateTime::epoch();
};

}
}
