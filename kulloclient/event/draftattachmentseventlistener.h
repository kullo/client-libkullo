/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
