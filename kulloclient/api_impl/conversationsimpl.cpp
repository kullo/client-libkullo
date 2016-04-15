/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/conversationsimpl.h"

#include <algorithm>
#include <boost/optional.hpp>

#include "kulloclient/api/DateTime.h"
#include "kulloclient/api_impl/addressimpl.h"
#include "kulloclient/api_impl/debug.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/event/conversationaddedevent.h"
#include "kulloclient/event/conversationremovedevent.h"
#include "kulloclient/event/sendapieventsevent.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/formatstring.h"
#include "kulloclient/util/misc.h"

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
        convForParticipants_[dao->participants()] = dao->id();
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
        auto dao = Dao::ConversationDao(sessionData_->dbSession_);
        dao.setParticipants(partString);
        dao.save();

        convId = dao.id();

        sessionListener_->internalEvent(
                    std::make_shared<Event::ConversationAddedEvent>(convId));
    }
    return convId;
}

void ConversationsImpl::remove(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto daoIter = conversations_.find(convId);
    if (daoIter != conversations_.end())
    {
        auto &dao = daoIter->second;

        const auto participants = dao.participants();
        auto countErased = convForParticipants_.erase(participants);
        kulloAssert(countErased > 0);

        dao.deletePermanently();
        conversations_.erase(daoIter);

        sessionListener_->internalEvent(
                    std::make_shared<Event::SendApiEventsEvent>(
                        Event::ApiEvents{
                            Api::Event(Api::EventType::ConversationRemoved,
                            convId, -1, -1)
                        }));
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
            ? iter->second.loadMessageCount(Dao::MessageState::Any)
            : 0;
}

int32_t ConversationsImpl::unreadMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessageState::Unread)
            : 0;
}

int32_t ConversationsImpl::undoneMessages(int64_t convId)
{
    kulloAssert(convId >= Kullo::ID_MIN && convId <= Kullo::ID_MAX);

    auto iter = conversations_.find(convId);
    return (iter != conversations_.end())
            ? iter->second.loadMessageCount(Dao::MessageState::Undone)
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

Event::ApiEvents ConversationsImpl::conversationAdded(uint64_t convId)
{
    auto dao = Dao::ConversationDao::load(convId, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError(
                "ConversationsImpl::conversationAdded");

    convForParticipants_[dao->participants()] = convId;
    conversations_.emplace(convId, *dao);

    return {Api::Event(Api::EventType::ConversationAdded, convId, -1, -1)};
}

Event::ApiEvents ConversationsImpl::conversationModified(uint64_t convId)
{
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
        events.emplace(Api::EventType::LatestSenderChanged, convId, -1, -1);
    }
    return events;
}

Event::ApiEvents ConversationsImpl::messageAdded(int64_t convId, int64_t msgId)
{
    K_UNUSED(msgId);

    // Simple hack: throw away cache data causing latestMessageTimestamp()
    // to reload next time it is called
    latestMessageTimestampsCache_.erase(convId);
    return {{}};
}

Event::ApiEvents ConversationsImpl::messageRemoved(int64_t convId, int64_t msgId)
{
    K_UNUSED(msgId);

    // Simple hack: throw away cache data causing latestMessageTimestamp()
    // to reload next time it is called
    latestMessageTimestampsCache_.erase(convId);
    return {{}};
}

Event::ApiEvents ConversationsImpl::conversationRemoved(int64_t convId)
{
    Event::ApiEvents result;

    auto daoIter = conversations_.find(convId);
    if (daoIter != conversations_.end())
    {
        auto &dao = daoIter->second;
        const auto participants = dao.participants();

        {
            auto countErased = convForParticipants_.erase(participants);
            kulloAssert(countErased > 0);
        }

        {
            // Those keys do not necessarily exist
            latestMessageTimestampsCache_.erase(convId);
        }

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
    auto iter = convForParticipants_.find(participants);
    return iter != convForParticipants_.end() ? iter->second : -1;
}

}
}
