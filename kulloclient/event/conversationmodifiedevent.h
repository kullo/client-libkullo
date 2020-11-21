/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
