/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
