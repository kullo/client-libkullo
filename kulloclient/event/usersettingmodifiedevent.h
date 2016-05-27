/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
