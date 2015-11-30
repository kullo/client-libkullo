/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/messagesevent.h"
#include "kulloclient/event/messageseventlistener.h"

namespace Kullo {
namespace Event {

class MessageModifiedEvent : public MessagesEvent
{
public:
    MessageModifiedEvent(int64_t convId, int64_t msgId)
        : convId_(convId),
          msgId_(msgId)
    {}

protected:
    ApiEvents notifyConcrete(MessagesEventListener &listener) override
    {
        return listener.messageModified(convId_, msgId_);
    }

private:
    int64_t convId_;
    int64_t msgId_;
};

}
}
