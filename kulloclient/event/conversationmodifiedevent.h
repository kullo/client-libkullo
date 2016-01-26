/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/conversationsevent.h"
#include "kulloclient/event/conversationseventlistener.h"

namespace Kullo {
namespace Event {

class ConversationModifiedEvent : public ConversationsEvent
{
public:
    ConversationModifiedEvent(int64_t convId)
        : convId_(convId)
    {}

protected:
    ApiEvents notifyConcrete(ConversationsEventListener &listener) override
    {
        return listener.conversationModified(convId_);
    }

private:
    int64_t convId_;
};

}
}
