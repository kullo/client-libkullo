/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
