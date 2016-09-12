/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Conversations.h"
#include "kulloclient/api/DateTime.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/event/conversationseventlistener.h"
#include "kulloclient/event/messageaddedevent.h"
#include "kulloclient/event/removaleventlistener.h"

namespace Kullo {
namespace ApiImpl {

class ConversationsImpl :
        public Api::Conversations,
        public Event::ConversationsEventListener,
        public Event::MessagesEventListener,
        public Event::RemovalEventListener
{
public:
    ConversationsImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    // Api::Conversations

    std::vector<int64_t> all() override;

    int64_t get(
            const std::unordered_set<std::shared_ptr<Api::Address>> &participants
            ) override;

    int64_t add(
            const std::unordered_set<std::shared_ptr<Api::Address>> &participants
            ) override;

    void triggerRemoval(int64_t convId) override;

    std::unordered_set<std::shared_ptr<Api::Address>> participants(
            int64_t convId) override;

    int32_t totalMessages(int64_t convId) override;

    int32_t unreadMessages(int64_t convId) override;

    int32_t undoneMessages(int64_t convId) override;

    Api::DateTime latestMessageTimestamp(int64_t convId) override;

    // Event::ConversationsEventListener

    Event::ApiEvents conversationAdded(uint64_t convId) override;
    Event::ApiEvents conversationModified(uint64_t convId) override;

    // Event::MessagesEventListener

    Event::ApiEvents messageAdded(int64_t convId, int64_t msgId) override;

    // Event::RemovalEventListener

    Event::ApiEvents messageRemoved(int64_t convId, int64_t msgId) override;
    Event::ApiEvents conversationWillBeRemoved(int64_t convId) override;

private:
    std::string participantsToString(
            const std::unordered_set<std::shared_ptr<Api::Address>> &participants);
    int64_t get(const std::string &participants);

    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::map<std::string, int64_t> participantsToConversationId_;
    std::map<int64_t, Dao::ConversationDao> conversations_;
    std::map<int64_t, Api::DateTime> latestMessageTimestampsCache_;
};

}
}
