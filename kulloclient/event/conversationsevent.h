/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class ConversationsEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.conversationsEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(ConversationsEventListener &listener) = 0;
};

}
}
