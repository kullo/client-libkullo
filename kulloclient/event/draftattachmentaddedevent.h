/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
