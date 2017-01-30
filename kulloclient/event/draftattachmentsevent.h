/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class DraftAttachmentsEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.draftAttachmentsEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(DraftAttachmentsEventListener &listener) = 0;
};

}
}
