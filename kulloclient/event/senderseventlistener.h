/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class SendersEventListener
{
public:
    virtual ApiEvents senderAdded(int64_t msgId) = 0;
};

}
}
