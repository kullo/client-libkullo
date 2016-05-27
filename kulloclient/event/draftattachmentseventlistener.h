/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/data/draftattachment.h"
#include "kulloclient/event/definitions.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Event {

class DraftAttachmentsEventListener
{
public:
    virtual ~DraftAttachmentsEventListener() = default;
    virtual ApiEvents draftAttachmentAdded(const Data::DraftAttachment &attachment) = 0;
    virtual ApiEvents draftAttachmentRemoved(id_type convId, id_type attId) = 0;
};

}
}
