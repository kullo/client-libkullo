/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/usersettingsdao.h"

#include <smartsqlite/connection.h>
#include <smartsqlite/exceptions.h>

#include "kulloclient/crypto/hasher.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/base64.h"

namespace Kullo {
namespace Dao {

/*
 * Principles
 *
 * * local changes go to local_value
 * * remote_value holds the server-side value
 * * when there's both a server-side and a client-side change, the server wins
 * * after syncing, local_value is empty because it has either been pushed to
 *   the server or been overwritten
 */

namespace {

std::string KEY_NAME = "name";
std::string KEY_ORGANIZATION = "organization";
std::string KEY_AVATAR_MIME_TYPE = "avatar_type";
std::string KEY_AVATAR_DATA = "avatar_data";
std::string KEY_FOOTER = "footer";
std::string KEY_NEXT_MASTER_KEY_BACKUP_REMINDER = "mk_backup_reminder";

}

UserSettingsDao::UserSettingsDao(const Db::SharedSessionPtr &session)
    : session_(session)
{
    kulloAssert(session_);
}

void UserSettingsDao::load(
        Util::UserSettings &userSettings,
        const boost::optional<std::string> &key)
{
    std::string sql =
            "SELECT key, coalesce(local_value, remote_value, '') AS value "
            "FROM usersettings ";
    if (key) sql += "WHERE key = :key ";

    auto stmt = session_->prepare(sql);

    if (key) stmt.bind(":key", *key);

    for (const auto &row : stmt)
    {
        auto key = row.get<std::string>("key");
        auto value = row.get<std::string>("value");

        if (key == KEY_NAME) userSettings.name = value;
        else if (key == KEY_ORGANIZATION) userSettings.organization = value;
        else if (key == KEY_AVATAR_MIME_TYPE) userSettings.avatarMimeType = value;
        else if (key == KEY_AVATAR_DATA) userSettings.avatarData = Util::Base64::decode(value);
        else if (key == KEY_FOOTER) userSettings.footer = value;
        else if (key == KEY_NEXT_MASTER_KEY_BACKUP_REMINDER)
        {
            auto reminder = (value.empty())
                    ? boost::none
                    : boost::make_optional(Util::DateTime(value));
            userSettings.nextMasterKeyBackupReminder = reminder;
        }
    }
}

void UserSettingsDao::setName(const std::string &value)
{
    setLocalValue(KEY_NAME, value);
}

void UserSettingsDao::setOrganization(const std::string &value)
{
    setLocalValue(KEY_ORGANIZATION, value);
}

void UserSettingsDao::setAvatarMimeType(const std::string &value)
{
    setLocalValue(KEY_AVATAR_MIME_TYPE, value);
}

void UserSettingsDao::setAvatarData(const std::vector<unsigned char> &value)
{
    setLocalValue(KEY_AVATAR_DATA, Util::Base64::encode(value));
}

void UserSettingsDao::setFooter(const std::string &value)
{
    setLocalValue(KEY_FOOTER, value);
}

void UserSettingsDao::setNextMasterKeyBackupReminder(
        const boost::optional<Util::DateTime> &value)
{
    auto str = (value) ? value->toString() : "";
    setLocalValue(KEY_NEXT_MASTER_KEY_BACKUP_REMINDER, str);
}

std::vector<Protocol::ProfileInfo> UserSettingsDao::localChanges()
{
    std::vector<Protocol::ProfileInfo> result;
    auto stmt = session_->prepare(
                "SELECT key, local_value, last_modified "
                "FROM usersettings "
                "WHERE local_value IS NOT NULL "
                "AND ((remote_value IS NULL) OR (local_value != remote_value)) ");

    for (const auto &row : stmt)
    {
        Protocol::ProfileInfo info;
        info.key = row.get<std::string>("key");
        info.value = row.get<std::string>("local_value");
        info.lastModified = row.get<lastModified_type>("last_modified");
        result.push_back(std::move(info));
    }
    return result;
}

void UserSettingsDao::setRemoteValue(Protocol::ProfileInfo newValue)
{
    auto savepoint = "usersettings-setRemoteValue-" + newValue.key;
    session_->savepoint(savepoint);

    auto stmt = session_->prepare(
                "UPDATE usersettings "
                "SET local_value = NULL, remote_value = :remote_value, "
                "last_modified = :last_modified "
                "WHERE key = :key AND last_modified != :last_modified ");
    stmt.bind(":key", newValue.key);
    stmt.bind(":remote_value", newValue.value);
    stmt.bind(":last_modified", newValue.lastModified);

    try
    {
        stmt.execWithoutResult();

        stmt = session_->prepare("SELECT changes()");
        auto changes = stmt.execWithSingleResult().get<int>(0);

        if (changes == 0)
        {
            // Use OR IGNORE because the previous query could have failed not
            // because there was no row but because last_modified was unchanged,
            // which is a case where we don't want to change the row.
            stmt = session_->prepare(
                            "INSERT OR IGNORE INTO usersettings "
                            "(key, remote_value, last_modified) "
                            "VALUES (:key, :remote_value, :last_modified) ");
            stmt.bind(":key", newValue.key);
            stmt.bind(":remote_value", newValue.value);
            stmt.bind(":last_modified", newValue.lastModified);
            stmt.execWithoutResult();
        }
    }
    catch (...)
    {
        session_->rollbackToSavepoint(savepoint);
        throw;
    }

    session_->releaseSavepoint(savepoint);
}

void UserSettingsDao::setLocalValue(
        const std::string &key, const std::string &localValue)
{
    auto savepoint = "usersettings-setLocalValue-" + key;
    session_->savepoint(savepoint);

    auto stmt = session_->prepare(
                "UPDATE usersettings "
                "SET local_value = :local_value "
                "WHERE key = :key ");
    stmt.bind(":key", key);
    stmt.bind(":local_value", localValue);

    try
    {
        stmt.execWithoutResult();

        stmt = session_->prepare("SELECT changes()");
        auto changes = stmt.execWithSingleResult().get<int>(0);

        if (changes == 0)
        {
            stmt = session_->prepare(
                            "INSERT INTO usersettings (key, local_value) "
                            "VALUES (:key, :local_value) ");
            stmt.bind(":key", key);
            stmt.bind(":local_value", localValue);
            stmt.execWithoutResult();
        }
    }
    catch (...)
    {
        session_->rollbackToSavepoint(savepoint);
        throw;
    }

    session_->releaseSavepoint(savepoint);
}

}
}
