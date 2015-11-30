/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/UserSettings.h"

#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace ApiImpl {

class UserSettingsImpl : public Api::UserSettings
{
public:
    UserSettingsImpl(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey);

    std::shared_ptr<Api::Address> address() override;
    std::shared_ptr<Api::MasterKey> masterKey() override;
    std::string name() override;
    void setName(const std::string &name) override;
    std::string organization() override;
    void setOrganization(const std::string &organization) override;
    std::string footer() override;
    void setFooter(const std::string &footer) override;
    std::string avatarMimeType() override;
    void setAvatarMimeType(const std::string &mimeType) override;
    std::vector<uint8_t> avatar() override;
    void setAvatar(const std::vector<uint8_t> &avatar) override;
    bool keyBackupConfirmed() override;
    void setKeyBackupConfirmed() override;
    boost::optional<Api::DateTime> keyBackupDontRemindBefore() override;
    void setKeyBackupDontRemindBefore(
            const boost::optional<Api::DateTime> &dontRemindBefore) override;

    Util::UserSettings userSettings() const;

private:
    Util::UserSettings userSettings_;
};

}
}
