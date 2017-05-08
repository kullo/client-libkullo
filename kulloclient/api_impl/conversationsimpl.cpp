/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/conversationsimpl.h"

#include <algorithm>
#include <boost/optional.hpp>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/api_impl/addressimpl.h"
#include "kulloclient/api_impl/debug.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/event/conversationaddedevent.h"
#include "kulloclient/event/conversationwillberemovedevent.h"
#include "kulloclient/event/sendapieventsevent.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/strings.h"

namespace Kullo {
namespace ApiImpl {

ConversationsImpl::ConversationsImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
{
    auto daos = Dao::ConversationDao::all(sessionData_->dbSession_);
    while (const auto &dao = daos->next())
    {
        participantsToConversationId_[dao->participants()] = dao->id();
        conversations_.emplace_hint(conversations_.end(), dao->id(), *dao);
    }
}

std::vector<int64_t> ConversationsImpl::all()
{
    std::vector<int64_t> result;
    for (auto &conv : conversations_)
    {
        result.push_back(conv.first);
    }
    return result;
}

int64_t ConversationsImpl::get(
        const std::unordered_set<std::shared_ptr<Api::Address>> &participants)
{
    kulloAssert(participants.size());

    return get(participantsToString(participants));
}

int64_t ConversationsImpl::add(
        const std::unordered_set<std::shared_ptr<Api::Address>> &participants)
{
    kulloAssert(participants.size());

    // Ensure that user is not sending message to himself
    const auto currentUserAddress = sessionData_->userSettings_->address();
    for (const auto &participant : participants)
    {
        kulloAssert(!participant->isEqualTo(currentUserAddress));
    }

    auto partString = participantsToString(participants);
    auto convId = get(partString);
    if (convId == -1)
    {
        SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
        auto dao = Dao::ConversationDao(sessionData_->dbSession_);
        dao.setParticipants(partString);
        dao.save();
        tx.commit();

        convId = dao.id();

        sessionListener_->internalEvent(
                    std::make_shared<Event::ConversationAddedEvent>(convId));
    }
    return convId;
}

void ConversationsImpl::triggerRemoval(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);
    auto daoIter = conversations_.find(convId);
    if (daoIter != conversations_.end())
    {
        sessionListener_->internalEvent(
                    std::make_shared<Event::ConversationWillBeRemovedEvent>(convId));
    }
}

std::unordered_set<std::shared_ptr<Api::Address>> ConversationsImpl::participants(
        int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    std::unordered_set<std::shared_ptr<Api::Address>> participants;

    auto iter = conversations_.find(convId);
    if (iter != conversations_.end())
    {
        for (const auto &part : iter->second.participantsList())
        {
            participants.emplace(std::make_shared<AddressImpl>(part));
        }
    }
    return participants;
}

int32_t ConversationsImpl::totalMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessagesFilter::Any)
            : 0;
}

int32_t ConversationsImpl::unreadMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessagesFilter::Unread)
            : 0;
}

int32_t ConversationsImpl::undoneMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessagesFilter::Undone)
            : 0;
}

int32_t ConversationsImpl::incomingMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessagesFilter::Incoming)
            : 0;
}

int32_t ConversationsImpl::outgoingMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessagesFilter::Outgoing)
            : 0;
}

Api::DateTime ConversationsImpl::latestMessageTimestamp(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    // optional to allow for later initialization (DateTime has no default ctor)
    boost::optional<Api::DateTime> result;

    auto cacheIter = latestMessageTimestampsCache_.find(convId);
    if (cacheIter != latestMessageTimestampsCache_.end())
    {
        result = cacheIter->second;
    }
    else
    {
        auto iter = conversations_.find(convId);
        if (iter == conversations_.end())
        {
            // conversation not found, return default
            result = Api::DateTime(Util::DateTime::epoch());
        }
        else
        {
            auto timestamp = iter->second.loadLastMessageTime();
            if (timestamp.empty())
            {
                // conversation found but empty, return emptyConversationTimestamp
                result = Api::Conversations::emptyConversationTimestamp();
            }
            else
            {
                // conversation found and nonempty, return last message time
                result = Api::DateTime(Util::DateTime(timestamp));
            }

            latestMessageTimestampsCache_.insert(std::make_pair(convId, *result));
        }
    }

    // result must have been set
    kulloAssert(result);
    return *result;
}

Event::ApiEvents ConversationsImpl::conversationAdded(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto dao = Dao::ConversationDao::load(convId, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError(
                "ConversationsImpl::conversationAdded");

    participantsToConversationId_[dao->participants()] = convId;
    conversations_.emplace(convId, *dao);

    return {Api::Event(Api::EventType::ConversationAdded, convId, -1, -1)};
}

Event::ApiEvents ConversationsImpl::conversationModified(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto dao = Dao::ConversationDao::load(convId, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError(
                "ConversationsImpl::conversationModified");

    auto pos = conversations_.find(convId);
    if (pos != conversations_.end())
    {
        conversations_.emplace_hint(conversations_.erase(pos), convId, *dao);
    }
    else
    {
        conversations_.emplace(convId, *dao);
    }

    auto oldTimestamp = latestMessageTimestamp(convId);
    latestMessageTimestampsCache_.erase(convId);
    auto newTimestamp = latestMessageTimestamp(convId);

    Event::ApiEvents events;
    events.emplace(Api::EventType::ConversationChanged, convId, -1, -1);
    if (oldTimestamp != newTimestamp)
    {
        events.emplace(Api::EventType::ConversationLatestMessageTimestampChanged, convId, -1, -1);
    }
    return events;
}

Event::ApiEvents ConversationsImpl::messageAdded(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);
    K_UNUSED(msgId);
    return {{}};
}

Event::ApiEvents ConversationsImpl::messageRemoved(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);
    K_UNUSED(msgId);
    return {{}};
}

Event::ApiEvents ConversationsImpl::conversationWillBeRemoved(int64_t convId)
{
    Event::ApiEvents result;

    auto daoIter = conversations_.find(convId);
    if (daoIter != conversations_.end())
    {
        auto &dao = daoIter->second;
        const auto participants = dao.participants();

        auto countErased = participantsToConversationId_.erase(participants);
        kulloAssert(countErased > 0);

        // does not necessarily exist
        latestMessageTimestampsCache_.erase(convId);

        SmartSqlite::ScopedTransaction tx(sessionData_->dbSession_, SmartSqlite::Immediate);
        dao.deletePermanently();
        tx.commit();
        conversations_.erase(daoIter);

        result.emplace(Api::EventType::ConversationRemoved,
                       convId, -1, -1);
    }

    return result;
}

std::string ConversationsImpl::participantsToString(
        const std::unordered_set<std::shared_ptr<Api::Address>> &participants)
{
    std::vector<std::string> partVec;
    for (const auto &part : participants) partVec.push_back(part->toString());

    // Sort and deduplicate addresses.
    // For unique(), the container must be sorted first!
    std::sort(partVec.begin(), partVec.end());
    auto uniqueEnd = std::unique(partVec.begin(), partVec.end());

    return Util::join(partVec.begin(), uniqueEnd, ",");
}

int64_t ConversationsImpl::get(const std::string &participants)
{
    auto iter = participantsToConversationId_.find(participants);
    return iter != participantsToConversationId_.end() ? iter->second : -1;
}

}
}
