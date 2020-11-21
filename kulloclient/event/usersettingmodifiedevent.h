/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <string>

#include "kulloclient/event/usersettingsevent.h"

namespace Kullo {
namespace Event {

class UserSettingModifiedEvent : public UserSettingsEvent
{
public:
    UserSettingModifiedEvent(const std::string &key)
        : key_(key)
    {}

protected:
    ApiEvents notifyConcrete(UserSettingsEventListener &listener) override
    {
        return listener.userSettingModified(key_);
    }

private:
    std::string key_;
};

}
}
