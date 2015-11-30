/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class RemovalEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.removalEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(RemovalEventListener &listener) = 0;
};

}
}
