/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class SendersEventListener
{
public:
    virtual ~SendersEventListener() = default;
    virtual ApiEvents senderAdded(int64_t msgId) = 0;
};

}
}
