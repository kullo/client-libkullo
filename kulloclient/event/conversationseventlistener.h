/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class ConversationsEventListener
{
public:
    virtual ApiEvents conversationAdded(uint64_t convId) = 0;
    virtual ApiEvents conversationModified(uint64_t convId) = 0;
};

}
}
