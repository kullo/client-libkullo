/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class MessagesEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.messagesEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(MessagesEventListener &listener) = 0;
};

}
}
