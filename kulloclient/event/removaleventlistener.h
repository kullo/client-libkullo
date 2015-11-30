/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Event {

class RemovalEventListener
{
public:
    // "{{}}" forces initializer list constructor
    // http://stackoverflow.com/questions/26947704/implicit-conversion-failure-from-initializer-list
    virtual ApiEvents conversationRemoved(int64_t convId)
    {
        K_UNUSED(convId);
        return {{}};
    }

    virtual ApiEvents messageRemoved(int64_t convId, int64_t msgId)
    {
        K_UNUSED(convId);
        K_UNUSED(msgId);
        return {{}};
    }

    virtual ApiEvents draftRemoved(int64_t convId)
    {
        K_UNUSED(convId);
        return {{}};
    }
};

}
}
