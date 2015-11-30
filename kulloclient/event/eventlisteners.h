/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/event/conversationseventlistener.h"
#include "kulloclient/event/messageseventlistener.h"
#include "kulloclient/event/messageattachmentseventlistener.h"
#include "kulloclient/event/draftseventlistener.h"
#include "kulloclient/event/draftattachmentseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"
#include "kulloclient/event/senderseventlistener.h"

namespace Kullo {
namespace Event {

class EventListeners
{
public:
    virtual ConversationsEventListener &conversationsEventListener() const = 0;
    virtual MessagesEventListener &messagesEventListener() const = 0;
    virtual SendersEventListener &sendersEventListener() const = 0;
    virtual MessageAttachmentsEventListener &messageAttachmentsEventListener() const = 0;
    virtual DraftsEventListener &draftsEventListener() const = 0;
    virtual DraftAttachmentsEventListener &draftAttachmentsEventListener() const = 0;
    virtual RemovalEventListener &removalEventListener() const = 0;
};

}
}
