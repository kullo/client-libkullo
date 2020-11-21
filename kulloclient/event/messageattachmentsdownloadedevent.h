/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/messageattachmentsevent.h"
#include "kulloclient/event/messageattachmentseventlistener.h"

namespace Kullo {
namespace Event {

class MessageAttachmentsDownloadedEvent : public MessageAttachmentsEvent
{
public:
    MessageAttachmentsDownloadedEvent(int64_t msgId)
        : msgId_(msgId)
    {}

protected:
    ApiEvents notifyConcrete(MessageAttachmentsEventListener &listener) override
    {
        return listener.messageAttachmentsDownloaded(msgId_);
    }

private:
    int64_t msgId_;
};

}
}
