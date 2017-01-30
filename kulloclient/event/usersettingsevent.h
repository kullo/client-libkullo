/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class UserSettingsEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.userSettingsEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(UserSettingsEventListener &listener) = 0;
};

}
}
