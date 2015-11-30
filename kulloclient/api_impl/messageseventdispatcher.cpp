/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/messageseventdispatcher.h"

#include "kulloclient/api_impl/sessionimpl.h"

namespace Kullo {
namespace ApiImpl {

MessagesEventDispatcher::MessagesEventDispatcher(SessionImpl &session)
    : session_(session)
{}

Event::ApiEvents MessagesEventDispatcher::messageAdded(
        int64_t convId, int64_t msgId)
{
    auto result = session_.messages_->messageAdded(convId, msgId);
    auto convResult = session_.conversations_->messageAdded(convId, msgId);
    auto attResult = session_.messageAttachments_->messageAdded(convId, msgId);
    result.insert(convResult.begin(), convResult.end());
    result.insert(attResult.begin(), attResult.end());
    return result;
}

Event::ApiEvents MessagesEventDispatcher::messageModified(
        int64_t convId, int64_t msgId)
{
    auto result = session_.messages_->messageModified(convId, msgId);
    auto convResult = session_.conversations_->messageModified(convId, msgId);
    auto attResult = session_.messageAttachments_->messageModified(convId, msgId);
    result.insert(convResult.begin(), convResult.end());
    result.insert(attResult.begin(), attResult.end());
    return result;
}

}
}
