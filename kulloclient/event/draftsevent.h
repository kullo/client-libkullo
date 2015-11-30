/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class DraftsEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.draftsEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(DraftsEventListener &listener) = 0;
};

}
}
