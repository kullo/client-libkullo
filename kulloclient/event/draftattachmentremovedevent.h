/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
