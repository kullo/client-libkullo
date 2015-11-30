/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/conversationsevent.h"
#include "kulloclient/event/conversationseventlistener.h"

namespace Kullo {
namespace Event {

class ConversationAddedEvent : public ConversationsEvent
{
public:
    ConversationAddedEvent(int64_t convId)
        : convId_(convId)
    {}

protected:
    ApiEvents notifyConcrete(ConversationsEventListener &listener) override
    {
        return listener.conversationAdded(convId_);
    }

private:
    int64_t convId_;
};

}
}
