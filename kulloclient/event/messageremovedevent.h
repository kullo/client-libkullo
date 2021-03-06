/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/removalevent.h"

namespace Kullo {
namespace Event {

class MessageRemovedEvent : public RemovalEvent
{
public:
    MessageRemovedEvent(int64_t convId, int64_t msgId)
        : convId_(convId),
          msgId_(msgId)
    {}

protected:
    ApiEvents notifyConcrete(RemovalEventListener &listener) override
    {
        return listener.messageRemoved(convId_, msgId_);
    }

private:
    int64_t convId_;
    int64_t msgId_;
};

}
}
