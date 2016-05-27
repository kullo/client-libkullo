/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/UserSettings.h"
#include "kulloclient/dao/usersettingsdao.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/event/usersettingseventlistener.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace ApiImpl {

class UserSettingsImpl :
        public Api::UserSettings,
        public Event::UserSettingsEventListener
{
public:
    UserSettingsImpl(
            const Db::SharedSessionPtr &dbSession,
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey);

    // Api::UserSettings

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
    boost::optional<Api::DateTime> nextMasterKeyBackupReminder() override;
    void setNextMasterKeyBackupReminder(
            const boost::optional<Api::DateTime> &reminderDate) override;

    // Event::UserSettingsEventListener

    Event::ApiEvents userSettingModified(const std::string &key) override;

    Util::UserSettings userSettings() const;

private:
    Util::UserSettings userSettings_;
    Dao::UserSettingsDao dao_;
};

}
}
