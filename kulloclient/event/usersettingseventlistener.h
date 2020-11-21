/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
