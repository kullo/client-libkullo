/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/usersettingsimpl.h"

#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/api_impl/MasterKey.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace ApiImpl {

UserSettingsImpl::UserSettingsImpl(
        const Db::SharedSessionPtr &dbSession,
        const Util::Credentials &credentials)
    : userSettings_(Util::UserSettings(credentials))
    , dbSession_(dbSession)
    , dao_(dbSession)
{
    dao_.load(userSettings_);
}

Api::Address UserSettingsImpl::address()
{
    return Api::Address(*userSettings_.credentials.address);
}

Api::MasterKey UserSettingsImpl::masterKey()
{
    return Api::MasterKey(*userSettings_.credentials.masterKey);
}

std::string UserSettingsImpl::name()
{
    return userSettings_.name;
}

void UserSettingsImpl::setName(const std::string &name)
{
    userSettings_.name = name;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setName(name);
    tx.commit();
}

std::string UserSettingsImpl::organization()
{
    return userSettings_.organization;
}

void UserSettingsImpl::setOrganization(const std::string &organization)
{
    userSettings_.organization = organization;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setOrganization(organization);
    tx.commit();
}

std::string UserSettingsImpl::footer()
{
    return userSettings_.footer;
}

void UserSettingsImpl::setFooter(const std::string &footer)
{
    userSettings_.footer = footer;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setFooter(footer);
    tx.commit();
}

std::string UserSettingsImpl::avatarMimeType()
{
    return userSettings_.avatarMimeType;
}

void UserSettingsImpl::setAvatarMimeType(const std::string &mimeType)
{
    userSettings_.avatarMimeType = mimeType;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setAvatarMimeType(mimeType);
    tx.commit();
}

std::vector<uint8_t> UserSettingsImpl::avatar()
{
    return userSettings_.avatarData;
}

void UserSettingsImpl::setAvatar(const std::vector<uint8_t> &avatar)
{
    userSettings_.avatarData = avatar;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setAvatarData(avatar);
    tx.commit();
}

boost::optional<Api::DateTime> UserSettingsImpl::nextMasterKeyBackupReminder()
{
    if (!userSettings_.nextMasterKeyBackupReminder) return boost::none;

    const auto &dt = *userSettings_.nextMasterKeyBackupReminder;
    return Api::DateTime(
                dt.year(), dt.month(), dt.day(),
                dt.hour(), dt.minute(), dt.second(),
                dt.tzOffsetMinutes());
}

void UserSettingsImpl::setNextMasterKeyBackupReminder(
        const boost::optional<Api::DateTime> &reminderDate)
{
    if (!reminderDate)
    {
        userSettings_.nextMasterKeyBackupReminder = boost::none;
        SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
        dao_.setNextMasterKeyBackupReminder(boost::none);
        tx.commit();
        return;
    }

    auto &date = *reminderDate;
    Util::DateTime result{
        date.year, date.month, date.day,
        date.hour, date.minute, date.second,
        date.tzOffsetMinutes
    };
    userSettings_.nextMasterKeyBackupReminder = result;
    SmartSqlite::ScopedTransaction tx(dbSession_, SmartSqlite::Immediate);
    dao_.setNextMasterKeyBackupReminder(result);
    tx.commit();
}

Event::ApiEvents UserSettingsImpl::userSettingModified(const std::string &key)
{
    dao_.load(userSettings_, key);
    return {Api::Event(Api::EventType::UserSettingsChanged, -1, -1, -1)};
}

Util::UserSettings UserSettingsImpl::userSettings() const
{
    return userSettings_;
}

}
}
