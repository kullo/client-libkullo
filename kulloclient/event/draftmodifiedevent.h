/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
