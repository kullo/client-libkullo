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
        const std::shared_ptr<Api::Address> &address,
        const std::shared_ptr<Api::MasterKey> &masterKey)
{
    kulloAssert(address);
    kulloAssert(masterKey);
    userSettings_.address =
            std::make_shared<Util::KulloAddress>(address->toString());
    userSettings_.masterKey =
            std::make_shared<Util::MasterKey>(masterKey->dataBlocks());
}

std::shared_ptr<Api::Address> UserSettingsImpl::address()
{
    return Api::Address::create(userSettings_.address->toString());
}

std::shared_ptr<Api::MasterKey> UserSettingsImpl::masterKey()
{
    return Api::MasterKey::createFromDataBlocks(
                userSettings_.masterKey->dataBlocks());
}

std::string UserSettingsImpl::name()
{
    return userSettings_.name;
}

void UserSettingsImpl::setName(const std::string &name)
{
    userSettings_.name = name;
}

std::string UserSettingsImpl::organization()
{
    return userSettings_.organization;
}

void UserSettingsImpl::setOrganization(const std::string &organization)
{
    userSettings_.organization = organization;
}

std::string UserSettingsImpl::footer()
{
    return userSettings_.footer;
}

void UserSettingsImpl::setFooter(const std::string &footer)
{
    userSettings_.footer = footer;
}

std::string UserSettingsImpl::avatarMimeType()
{
    return userSettings_.avatarMimeType;
}

void UserSettingsImpl::setAvatarMimeType(const std::string &mimeType)
{
    userSettings_.avatarMimeType = mimeType;
}

std::vector<uint8_t> UserSettingsImpl::avatar()
{
    return userSettings_.avatarData;
}

void UserSettingsImpl::setAvatar(const std::vector<uint8_t> &avatar)
{
    userSettings_.avatarData = avatar;
}

bool UserSettingsImpl::keyBackupConfirmed()
{
    return userSettings_.masterKeyBackupConfirmed();
}

void UserSettingsImpl::setKeyBackupConfirmed()
{
    userSettings_.setMasterKeyBackupConfirmed();
}

boost::optional<Api::DateTime> UserSettingsImpl::keyBackupDontRemindBefore()
{
    const auto &dt = userSettings_.masterKeyBackupDontRemindBefore();
    if (dt.isNull()) return boost::none;
    return Api::DateTime(
                dt.year(), dt.month(), dt.day(),
                dt.hour(), dt.minute(), dt.second(),
                std::round(dt.tzOffset() / 60.0)
    );
}

void UserSettingsImpl::setKeyBackupDontRemindBefore(
        const boost::optional<Api::DateTime> &dontRemindBefore)
{
    Util::DateTime result;
    if (dontRemindBefore)
    {
        auto &drb = *dontRemindBefore;
        result = Util::DateTime(
            drb.year, drb.month, drb.day,
            drb.hour, drb.minute, drb.second,
            drb.tzOffsetMinutes * 60
        );
    }
    userSettings_.setMasterKeyBackupDontRemindBefore(result);
}

Util::UserSettings UserSettingsImpl::userSettings() const
{
    return userSettings_;
}

}
}
