/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/event.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Event {

/**
 * @brief Event that returns the events that have been given to it.
 *
 * The SendApiEventsEvent is necessary for synchronous model methods that want
 * to emit ApiEvents, similar to what InternalEvent handler methods can do.
 * InternalEvent handlers have a return value of type ApiEvents, which the
 * synchronous model methods lack (and which should not be introduced to not
 * duplicate event handling code in the API client).
 */
class SendApiEventsEvent : public Event
{
public:
    SendApiEventsEvent(ApiEvents events)
        : events_(std::move(events))
    {}

    /// Convenience constructor that stores an event set with a single entry
    SendApiEventsEvent(Api::Event event)
        : SendApiEventsEvent(ApiEvents{std::move(event)})
    {}

    ApiEvents notify(const EventListeners &listeners) override
    {
        K_UNUSED(listeners);
        return events_;
    }

private:
    ApiEvents events_;
};

}
}
