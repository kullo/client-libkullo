/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/sendersevent.h"
#include "kulloclient/event/senderseventlistener.h"

namespace Kullo {
namespace Event {

class SenderAddedEvent : public SendersEvent
{
public:
    SenderAddedEvent(int64_t msgId)
        : msgId_(msgId)
    {}

protected:
    ApiEvents notifyConcrete(SendersEventListener &listener) override
    {
        return listener.senderAdded(msgId_);
    }

private:
    int64_t msgId_;
};

}
}
