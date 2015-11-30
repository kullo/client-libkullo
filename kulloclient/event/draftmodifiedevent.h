/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/draftsevent.h"
#include "kulloclient/event/draftseventlistener.h"

namespace Kullo {
namespace Event {

class DraftModifiedEvent : public DraftsEvent
{
public:
    DraftModifiedEvent(int64_t convId)
        : convId_(convId)
    {}

protected:
    ApiEvents notifyConcrete(DraftsEventListener &listener) override
    {
        return listener.draftModified(convId_);
    }

private:
    int64_t convId_;
};

}
}
