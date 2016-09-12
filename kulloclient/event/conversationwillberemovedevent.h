/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/removalevent.h"

namespace Kullo {
namespace Event {

class ConversationWillBeRemovedEvent : public RemovalEvent
{
public:
    ConversationWillBeRemovedEvent(int64_t convId)
        : convId_(convId)
    {}

protected:
    ApiEvents notifyConcrete(RemovalEventListener &listener) override
    {
        return listener.conversationWillBeRemoved(convId_);
    }

private:
    int64_t convId_;
};

}
}
