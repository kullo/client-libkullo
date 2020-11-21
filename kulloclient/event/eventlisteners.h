/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/event/conversationseventlistener.h"
#include "kulloclient/event/messageseventlistener.h"
#include "kulloclient/event/messageattachmentseventlistener.h"
#include "kulloclient/event/draftseventlistener.h"
#include "kulloclient/event/draftattachmentseventlistener.h"
#include "kulloclient/event/removaleventlistener.h"
#include "kulloclient/event/senderseventlistener.h"
#include "kulloclient/event/usersettingseventlistener.h"

namespace Kullo {
namespace Event {

class EventListeners
{
public:
    virtual ~EventListeners() = default;
    virtual ConversationsEventListener &conversationsEventListener() const = 0;
    virtual MessagesEventListener &messagesEventListener() const = 0;
    virtual SendersEventListener &sendersEventListener() const = 0;
    virtual MessageAttachmentsEventListener &messageAttachmentsEventListener() const = 0;
    virtual DraftsEventListener &draftsEventListener() const = 0;
    virtual DraftAttachmentsEventListener &draftAttachmentsEventListener() const = 0;
    virtual UserSettingsEventListener &userSettingsEventListener() const = 0;
    virtual RemovalEventListener &removalEventListener() const = 0;
};

}
}
