/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/event.h"

#include <ostream>

#include "kulloclient/api/Event.h"
#include "kulloclient/api/EventType.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"

std::ostream & operator<<(std::ostream &os, const Kullo::Api::EventType& t)
{
    std::string out;

    using EventType = Kullo::Api::EventType;
    switch (t) {
    case EventType::ConversationAdded:                   out = K_AS_STRING(EventType::ConversationAdded);                   break;
    case EventType::ConversationChanged:                 out = K_AS_STRING(EventType::ConversationChanged);                 break;
    case EventType::ConversationRemoved:                 out = K_AS_STRING(EventType::ConversationRemoved);                 break;
    case EventType::DraftStateChanged:                   out = K_AS_STRING(EventType::DraftStateChanged);                   break;
    case EventType::DraftTextChanged:                    out = K_AS_STRING(EventType::DraftTextChanged);                    break;
    case EventType::DraftAttachmentAdded:                out = K_AS_STRING(EventType::DraftAttachmentAdded);                break;
    case EventType::DraftAttachmentRemoved:              out = K_AS_STRING(EventType::DraftAttachmentRemoved);              break;
    case EventType::MessageAdded:                        out = K_AS_STRING(EventType::MessageAdded);                        break;
    case EventType::MessageDeliveryChanged:              out = K_AS_STRING(EventType::MessageDeliveryChanged);              break;
    case EventType::MessageStateChanged:                 out = K_AS_STRING(EventType::MessageStateChanged);                 break;
    case EventType::MessageAttachmentsDownloadedChanged: out = K_AS_STRING(EventType::MessageAttachmentsDownloadedChanged); break;
    case EventType::MessageRemoved:                      out = K_AS_STRING(EventType::MessageRemoved);                      break;
    case EventType::LatestSenderChanged:                 out = K_AS_STRING(EventType::LatestSenderChanged);                 break;
    default: kulloAssert(false);
    }

    return os << out;
}

std::ostream & operator<<(std::ostream &os, const Kullo::Api::Event& e)
{
    os << e.event << "["
       << "convId: " << e.conversationId <<  ", msgId: " << e.messageId << ", attId: " << e.attachmentId
       <<  "]";
    return os;
}
