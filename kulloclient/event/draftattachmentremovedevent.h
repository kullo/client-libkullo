/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/draftattachmentsevent.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Event {

class DraftAttachmentRemovedEvent : public DraftAttachmentsEvent
{
public:
    DraftAttachmentRemovedEvent(id_type convId, id_type attId)
        : convId_(convId)
        , attId_(attId)
    {}

protected:
    ApiEvents notifyConcrete(DraftAttachmentsEventListener &listener) override
    {
        return listener.draftAttachmentRemoved(convId_, attId_);
    }

private:
    id_type convId_, attId_;
};

}
}
