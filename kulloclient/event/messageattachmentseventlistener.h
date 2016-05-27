/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Event {

class MessageAttachmentsEventListener
{
public:
    virtual ~MessageAttachmentsEventListener() = default;

    // "{{}}" forces initializer list constructor
    // http://stackoverflow.com/questions/26947704/implicit-conversion-failure-from-initializer-list
    virtual ApiEvents messageAttachmentsDownloaded(uint64_t msgId)
    {
        K_UNUSED(msgId);
        return {{}};
    }
};

}
}
