/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>

#define EMIT0(event) if (event) event()
#define EMIT(event, ...) if (event) event(__VA_ARGS__)

namespace Kullo {
namespace Util {

/**
 * @brief Forward an event to another event lazily.
 *
 * Binds the event that is stored in the variable referenced by the argument.
 * This is different from simply assigning the event because this would evaluate
 * the right-hand side at the time of assignment and not at the time of
 * triggering the event.
 *
 * Example:
 *
 * events.fooEvent = foo;
 * events.barEvent = forwardEvent(events.fooEvent);
 * events.fooEvent = fooBar;
 * events.barEvent();  // triggers fooBar()
 *
 * Compare this to the naive implementation:
 *
 * events.fooEvent = foo;
 * events.barEvent = events.fooEvent;
 * events.fooEvent = fooBar;
 * events.barEvent();  // triggers foo()
 */
template <typename... Args>
std::function<void(Args...)>
forwardEvent(const std::function<void(Args...)> &event)
{
    // capture event by reference to achieve lazy evaluation
    return [&event](Args&&... args)
    {
        EMIT(event, std::forward<Args>(args)...);
    };
}

}
}
