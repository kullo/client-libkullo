/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/data/draftattachment.h"
#include "kulloclient/event/draftattachmentsevent.h"

namespace Kullo {
namespace Event {

class DraftAttachmentAddedEvent : public DraftAttachmentsEvent
{
public:
    DraftAttachmentAddedEvent(const Data::DraftAttachment &data)
        : data_(data)
    {}

protected:
    ApiEvents notifyConcrete(DraftAttachmentsEventListener &listener) override
    {
        return listener.draftAttachmentAdded(data_);
    }

private:
    Data::DraftAttachment data_;
};

}
}
