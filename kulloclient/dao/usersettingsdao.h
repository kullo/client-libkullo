/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/optional/optional_fwd.hpp>

#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/usersettings.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Dao {

class UserSettingsDao
{
public:
    explicit UserSettingsDao(const Db::SharedSessionPtr &session);

    /// Loads all settings from the DB into userSettings, overwriting all
    /// mutable fields.
    void load(
            Util::UserSettings &userSettings,
            const boost::optional<std::string> &key = boost::none);

    void setName(const std::string &value);
    void setOrganization(const std::string &value);
    void setAvatarMimeType(const std::string &value);
    void setAvatarData(const std::vector<unsigned char> &value);
    void setFooter(const std::string &value);
    void setNextMasterKeyBackupReminder(
            const boost::optional<Util::DateTime> &value);

    std::vector<Protocol::ProfileInfo> localChanges();
    void setRemoteValue(Protocol::ProfileInfo newValue);

private:
    void setLocalValue(const std::string &key, const std::string &localValue);

    Db::SharedSessionPtr session_;
};

}
}
