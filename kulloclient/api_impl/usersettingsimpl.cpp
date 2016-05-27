/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/usersettingsimpl.h"

#include <cmath>

#include "kulloclient/api/Address.h"
#include "kulloclient/api/DateTime.h"
#include "kulloclient/api/MasterKey.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace ApiImpl {

UserSettingsImpl::UserSettingsImpl(
        const Db::SharedSessionPtr &dbSession,
        const std::shared_ptr<Api::Address> &address,
        const std::shared_ptr<Api::MasterKey> &masterKey)
    : userSettings_(
          Util::UserSettings(
              Util::Credentials(
                  std::make_shared<Util::KulloAddress>(address->toString()),
                  std::make_shared<Util::MasterKey>(masterKey->dataBlocks()))))
    , dao_(dbSession)
{
    dao_.load(userSettings_);
}

std::shared_ptr<Api::Address> UserSettingsImpl::address()
{
    return Api::Address::create(userSettings_.credentials.address->toString());
}

std::shared_ptr<Api::MasterKey> UserSettingsImpl::masterKey()
{
    return Api::MasterKey::createFromDataBlocks(
                userSettings_.credentials.masterKey->dataBlocks());
}

std::string UserSettingsImpl::name()
{
    return userSettings_.name;
}

void UserSettingsImpl::setName(const std::string &name)
{
    userSettings_.name = name;
    dao_.setName(name);
}

std::string UserSettingsImpl::organization()
{
    return userSettings_.organization;
}

void UserSettingsImpl::setOrganization(const std::string &organization)
{
    userSettings_.organization = organization;
    dao_.setOrganization(organization);
}

std::string UserSettingsImpl::footer()
{
    return userSettings_.footer;
}

void UserSettingsImpl::setFooter(const std::string &footer)
{
    userSettings_.footer = footer;
    dao_.setFooter(footer);
}

std::string UserSettingsImpl::avatarMimeType()
{
    return userSettings_.avatarMimeType;
}

void UserSettingsImpl::setAvatarMimeType(const std::string &mimeType)
{
    userSettings_.avatarMimeType = mimeType;
    dao_.setAvatarMimeType(mimeType);
}

std::vector<uint8_t> UserSettingsImpl::avatar()
{
    return userSettings_.avatarData;
}

void UserSettingsImpl::setAvatar(const std::vector<uint8_t> &avatar)
{
    userSettings_.avatarData = avatar;
    dao_.setAvatarData(avatar);
}

boost::optional<Api::DateTime> UserSettingsImpl::nextMasterKeyBackupReminder()
{
    if (!userSettings_.nextMasterKeyBackupReminder) return boost::none;

    const auto &dt = *userSettings_.nextMasterKeyBackupReminder;
    if (dt.isNull()) return boost::none;

    return Api::DateTime(
                dt.year(), dt.month(), dt.day(),
                dt.hour(), dt.minute(), dt.second(),
                std::round(dt.tzOffset() / 60.0));
}

void UserSettingsImpl::setNextMasterKeyBackupReminder(
        const boost::optional<Api::DateTime> &reminderDate)
{
    if (!reminderDate)
    {
        userSettings_.nextMasterKeyBackupReminder = boost::none;
        dao_.setNextMasterKeyBackupReminder(boost::none);
        return;
    }

    Util::DateTime result;
    auto &date = *reminderDate;
    result = Util::DateTime(
        date.year, date.month, date.day,
        date.hour, date.minute, date.second,
        date.tzOffsetMinutes * 60
    );
    userSettings_.nextMasterKeyBackupReminder = result;
    dao_.setNextMasterKeyBackupReminder(result);
}

Event::ApiEvents UserSettingsImpl::userSettingModified(const std::string &key)
{
    dao_.load(userSettings_, key);
    return {{}};
}

Util::UserSettings UserSettingsImpl::userSettings() const
{
    return userSettings_;
}

}
}
