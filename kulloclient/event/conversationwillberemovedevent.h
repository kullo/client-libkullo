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
