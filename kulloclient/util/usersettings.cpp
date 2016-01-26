/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Util {

bool UserSettings::masterKeyBackupConfirmed() const
{
    return masterKeyBackupConfirmed_;
}

void UserSettings::setMasterKeyBackupConfirmed()
{
    if (masterKeyBackupConfirmed_) return;

    masterKeyBackupConfirmed_ = true;
    masterKeyBackupDontRemindBefore_ = DateTime();
}

DateTime UserSettings::masterKeyBackupDontRemindBefore() const
{
    return masterKeyBackupDontRemindBefore_;
}

void UserSettings::setMasterKeyBackupDontRemindBefore(const DateTime &dontRemindBefore)
{
    masterKeyBackupConfirmed_ = false;
    masterKeyBackupDontRemindBefore_ = dontRemindBefore;
}

void UserSettings::reset()
{
    address = nullptr;
    name.clear();
    organization.clear();
    avatarMimeType.clear();
    avatarData.clear();
    footer.clear();
    masterKey = nullptr;
    masterKeyBackupConfirmed_ = false;
    masterKeyBackupDontRemindBefore_ = DateTime::epoch();
}

}
}
