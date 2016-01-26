/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Event {

class MessagesEventListener
{
public:
    // "{{}}" forces initializer list constructor
    // http://stackoverflow.com/questions/26947704/implicit-conversion-failure-from-initializer-list
    virtual ApiEvents messageAdded(int64_t convId, int64_t msgId)
    {
        K_UNUSED(convId);
        K_UNUSED(msgId);
        return {{}};
    }

    virtual ApiEvents messageModified(int64_t convId, int64_t msgId)
    {
        K_UNUSED(convId);
        K_UNUSED(msgId);
        return {{}};
    }
};

}
}
