/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/definitions.h"

namespace Kullo {
namespace Event {

class ConversationsEventListener
{
public:
    virtual ~ConversationsEventListener() = default;
    virtual ApiEvents conversationAdded(int64_t convId) = 0;
    virtual ApiEvents conversationModified(int64_t convId) = 0;
};

}
}
