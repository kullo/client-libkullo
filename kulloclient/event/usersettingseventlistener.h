/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class UserSettingsEventListener
{
public:
    virtual ~UserSettingsEventListener() = default;
    virtual ApiEvents userSettingModified(const std::string &key) = 0;
};

}
}
