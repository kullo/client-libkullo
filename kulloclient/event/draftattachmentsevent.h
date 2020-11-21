/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/event.h"

namespace Kullo {
namespace Event {

class DraftAttachmentsEvent : public Event
{
public:
    ApiEvents notify(const EventListeners &listeners) override
    {
        return notifyConcrete(listeners.draftAttachmentsEventListener());
    }

protected:
    virtual ApiEvents notifyConcrete(DraftAttachmentsEventListener &listener) = 0;
};

}
}
