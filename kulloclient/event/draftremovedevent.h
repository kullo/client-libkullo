/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/removalevent.h"

namespace Kullo {
namespace Event {

class DraftRemovedEvent : public RemovalEvent
{
public:
    DraftRemovedEvent(int64_t convId)
        : convId_(convId)
    {}

protected:
    ApiEvents notifyConcrete(RemovalEventListener &listener) override
    {
        return listener.draftRemoved(convId_);
    }

private:
    int64_t convId_;
};

}
}
