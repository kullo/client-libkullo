/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/removaleventdispatcher.h"

#include "kulloclient/api_impl/sessionimpl.h"

namespace Kullo {
namespace ApiImpl {

RemovalEventDispatcher::RemovalEventDispatcher(SessionImpl &session)
    : session_(session)
{}

Event::ApiEvents RemovalEventDispatcher::conversationWillBeRemoved(int64_t convId)
{
    Event::ApiEvents result;
    auto messagesResult = session_.messages_->conversationWillBeRemoved(convId);
    auto draftsResult = session_.drafts_->conversationWillBeRemoved(convId);

    // the last call of conversationWillBeRemoved() must go to Conversations
    auto conversationsResult = session_.conversations_->conversationWillBeRemoved(convId);

    result.insert(messagesResult.begin(), messagesResult.end());
    result.insert(draftsResult.begin(), draftsResult.end());
    result.insert(conversationsResult.begin(), conversationsResult.end());
    return result;
}

Event::ApiEvents RemovalEventDispatcher::messageRemoved(int64_t convId, int64_t msgId)
{
    Event::ApiEvents result;
    auto messageAttachmentsResult = session_.messageAttachments_->messageRemoved(convId, msgId);
    auto messagesResult = session_.messages_->messageRemoved(convId, msgId);
    auto sendersResult = session_.senders_->messageRemoved(convId, msgId);
    auto conversationsResult = session_.conversations_->messageRemoved(convId, msgId);
    result.insert(messageAttachmentsResult.begin(), messageAttachmentsResult.end());
    result.insert(messagesResult.begin(), messagesResult.end());
    result.insert(sendersResult.begin(), sendersResult.end());
    result.insert(conversationsResult.begin(), conversationsResult.end());
    return result;
}

Event::ApiEvents RemovalEventDispatcher::draftRemoved(int64_t convId)
{
    return session_.draftAttachments_->draftRemoved(convId);
}

}
}
