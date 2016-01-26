/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/InternalEvent.h"
#include "kulloclient/event/definitions.h"
#include "kulloclient/event/eventlisteners.h"

namespace Kullo {
namespace Event {

class Event : public Api::InternalEvent
{
public:
    virtual ApiEvents notify(const EventListeners &listeners) = 0;
};

}
}
